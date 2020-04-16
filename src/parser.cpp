#include "parser.hpp"

// WHY DO I HAVE TO TYPE THIS TWICE!?
template<typename T, typename U>
bool handle_error (Writer<T>& lhs, Writer<U>& rhs) {
	if(not rhs.ok()){
		lhs.m_error.m_sub_errors.push_back(std::move(rhs.m_error));
		return true;
	}
	return false;
}

template<typename T, typename U>
bool handle_error (Writer<T>& lhs, Writer<U>&& rhs) {
	if(not rhs.ok()){
		lhs.m_error.m_sub_errors.push_back(std::move(rhs.m_error));
		return true;
	}
	return false;
}

Writer<std::unique_ptr<AST>> Parser::parse_top_level () {
	Writer<std::unique_ptr<AST>>  result = {
		{"Parse Error: Failed to parse top level program"}
	};

	auto e = std::make_unique<ASTDeclarationList>();

	while(not m_lexer->done()){
		auto declaration = parse_declaration();

		if(handle_error(result, declaration))
			return result;

		e->m_declarations.push_back(std::move(declaration.m_result));
	}

	return make_writer<std::unique_ptr<AST>>(std::move(e));
}

Writer<std::unique_ptr<AST>> Parser::parse_declaration () {
	Writer<std::unique_ptr<AST>> result = {
		{"Parse Error: Failed to parse declaration"}
	};

	Writer<Token const*> name = require(token_type::IDENTIFIER);

	if (handle_error(result, name)) return result;

	Writer<Token const*> type;

	if (handle_error(result, require(token_type::DECLARE_ASSIGN))) {

		if (handle_error(result, require(token_type::DECLARE)))
			return result;

		type = require(token_type::IDENTIFIER);
		if (handle_error(result, type))
			return result;

		if (handle_error(result, require(token_type::ASSIGN)))
			return result;

	}

	auto value = parse_expression();

	if(handle_error(result, value))
		return result;

	if(handle_error(result, require(token_type::SEMICOLON)))
		return result;

	auto p = std::make_unique<ASTDeclaration>();

	p->m_identifier = name.m_result->m_text;
	if(type.m_result) p->m_typename = type.m_result->m_text;
	p->m_value = std::move(value.m_result);

	return make_writer<std::unique_ptr<AST>>(std::move(p));
}

Writer<std::unique_ptr<AST>> Parser::parse_expression () {
	Writer<std::unique_ptr<AST>> result = {
		{"Parse Error: Failed to parse expression"}
	};

	auto number = require(token_type::NUMBER);

	if (not handle_error(result, number)) {
		auto e = std::make_unique<ASTNumberLiteral>();
		e->m_text = number.m_result->m_text;
		result = make_writer<std::unique_ptr<AST>>(std::move(e));

		return result;
	}

	auto identifier = require(token_type::IDENTIFIER);

	if(not handle_error(result, identifier)){
		auto e = std::make_unique<ASTIdentifier>();
		e->m_text = identifier.m_result->m_text;

		result = make_writer<std::unique_ptr<AST>>(std::move(e));

		return result;
	}

	return result;
}

Writer<Token const*> Parser::require (token_type t){
	/* got */
	Token const* current_token = &m_lexer->current_token();

	/* expected */
	if(current_token->m_type != t){

		std::stringstream ss;
		ss
			<< "Parse Error: @ "
			<< current_token->m_line0+1 << ":" << current_token->m_col0+1
			<< ": Expected "
			<< token_type_string[int(t)]
			<< " but got "
			<< token_type_string[int(current_token->m_type)]
			<< " instead";

		return {ParseError{ss.str()}};
	}

	m_lexer->advance();

	return make_writer(current_token);
}
