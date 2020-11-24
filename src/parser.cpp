#include "parser.hpp"

#include "utils/string_view.hpp"
#include "ast_allocator.hpp"

#include <sstream>
#include <utility>

#include <cassert>

// WHY DO I HAVE TO TYPE THIS TWICE!?
template <typename T, typename U>
bool handle_error(Writer<T>& lhs, Writer<U>& rhs) {
	if (not rhs.ok()) {
		lhs.m_error.m_sub_errors.push_back(std::move(rhs.m_error));
		return true;
	}
	return false;
}

template <typename T, typename U>
bool handle_error(Writer<T>& lhs, Writer<U>&& rhs) {
	if (not rhs.ok()) {
		lhs.m_error.m_sub_errors.push_back(std::move(rhs.m_error));
		return true;
	}
	return false;
}

ErrorReport make_expected_error(string_view expected, Token const* found_token) {
	std::stringstream ss;
	ss << "Parse Error: @ " << found_token->m_line0 + 1 << ":"
	   << found_token->m_col0 + 1 << ": Expected " << expected << " but got "
	   << token_string[int(found_token->m_type)] << ' '
	   << found_token->m_text << " instead";

	return ErrorReport {ss.str()};
}

Writer<Token const*> Parser::require(TokenTag expected_type) {
	Token const* current_token = &m_lexer->current_token();

	if (current_token->m_type != expected_type) {
		return {make_expected_error(
		    token_string[int(expected_type)], current_token)};
	}

	m_lexer->advance();

	return make_writer(current_token);
}

bool Parser::consume(TokenTag expected_type) {
	if (match(expected_type)) {
		m_lexer->advance();
		return true;
	}
	return false;
}

bool Parser::match(TokenTag expected_type) {
	Token const* current_token = &m_lexer->current_token();
	if (current_token->m_type != expected_type)
		return false;
	return true;
}

Writer<AST::AST*> Parser::parse_top_level() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse top level program"}};

	auto declarations = parse_declaration_list(TokenTag::END);

	if (handle_error(result, declarations))
		return result;

	auto e = m_ast_allocator->make<AST::DeclarationList>();

	e->m_declarations = std::move(declarations.m_result);

	return make_writer<AST::AST*>(e);
}

Writer<std::vector<AST::Declaration>>
Parser::parse_declaration_list(TokenTag terminator) {
	Writer<std::vector<AST::Declaration>> result = {
	    {"Parse Error: Failed to parse declaration list"}};

	std::vector<AST::Declaration> declarations;

	while (1) {
		auto p0 = peek();

		if (p0->m_type == terminator)
			break;

		if (p0->m_type == TokenTag::END) {
			result.m_error.m_sub_errors.push_back(
			    make_expected_error("a declaration", p0));

			result.m_error.m_sub_errors.push_back(
			    make_expected_error(token_string[int(terminator)], p0));

			return result;
		}

		auto declaration = parse_declaration();

		if (handle_error(result, declaration))
			return result;

		declarations.push_back(std::move(*declaration.m_result));
	}

	return make_writer(std::move(declarations));
}

Writer<std::vector<AST::AST*>> Parser::parse_expression_list(
    TokenTag delimiter, TokenTag terminator, bool allow_trailing_delimiter) {
	Writer<std::vector<AST::AST*>> result = {
	    {"Parse Error: Failed to parse expression list"}};

	std::vector<AST::AST*> expressions;

	if (peek()->m_type == terminator) {
		m_lexer->advance();
	} else {
		while (1) {
			auto p0 = peek();

			if (p0->m_type == TokenTag::END) {
				result.m_error.m_sub_errors.push_back(
				    {{"Found EOF while parsing an expression list"}});
				return result;
			}

			if (p0->m_type == terminator) {
				if (allow_trailing_delimiter) {
					m_lexer->advance();
					break;
				} else {
					result.m_error.m_sub_errors.push_back(
					    {{"Found a terminator after a delimiter in an "
					      "expression list "}});
					return result;
				}
			}

			auto expression = parse_expression();

			if (handle_error(result, expression)) {
				return result;
			} else {
				expressions.push_back(expression.m_result);
			}

			auto p1 = peek();

			if (p1->m_type == delimiter) {
				m_lexer->advance();
			} else if (p1->m_type == terminator) {
				m_lexer->advance();
				break;
			} else {
				result.m_error.m_sub_errors.push_back(
				    make_expected_error(token_string[(int)delimiter], p1));
				result.m_error.m_sub_errors.push_back(
				    make_expected_error(token_string[(int)terminator], p1));
				return result;
			}
		}
	}

	return make_writer(std::move(expressions));
}

Writer<AST::Declaration*> Parser::parse_declaration() {
	Writer<AST::Declaration*> result = {
	    {"Parse Error: Failed to parse declaration"}};

	Writer<Token const*> name = require(TokenTag::IDENTIFIER);

	if (handle_error(result, name))
		return result;

	Writer<AST::AST*> type;

	if (consume(TokenTag::DECLARE_ASSIGN)) {
	} else if (consume(TokenTag::DECLARE)) {

		type = parse_type_term();
		if (handle_error(result, type))
			return result;

		if (handle_error(result, require(TokenTag::ASSIGN)))
			return result;
	} else {
		result.m_error.m_sub_errors.push_back(
		    make_expected_error("':' or ':='", peek()));
	}

	auto value = parse_expression();

	if (handle_error(result, value))
		return result;

	if (handle_error(result, require(TokenTag::SEMICOLON)))
		return result;

	auto p = m_ast_allocator->make<AST::Declaration>();

	p->m_identifier_token = name.m_result;
	p->m_type_hint = type.m_result;
	p->m_value = value.m_result;

	return make_writer<AST::Declaration*>(p);
}

// These are important for infix expression parsing.
// TODO: clean up

struct binding_power {
	int left, right;
};

bool is_binary_operator(TokenTag t) {
	// TODO: fill out this table
	switch (t) {
	case TokenTag::PAREN_OPEN:   // a bit of a hack
	case TokenTag::BRACKET_OPEN: // a bit of a hack
	case TokenTag::POLY_OPEN:    // a bit of a hack
	case TokenTag::BRACE_OPEN:   // a bit of a hack

	case TokenTag::LT:
	case TokenTag::GT:
	case TokenTag::LTE:
	case TokenTag::GTE:
	case TokenTag::EQUAL:
	case TokenTag::NOT_EQUAL:
	case TokenTag::ASSIGN:
	case TokenTag::DOT:
	case TokenTag::PIZZA:
	case TokenTag::ADD:
	case TokenTag::SUB:
	case TokenTag::MUL:
	case TokenTag::DIV: // fallthrough
		return true;
	default:
		return false;
	}
}

binding_power binding_power_of(TokenTag t) {
	// TODO: fill out this table
	switch (t) {

	case TokenTag::ASSIGN:
		return {10, 11};
	case TokenTag::PIZZA:
		return {20, 21};
	case TokenTag::LT:
	case TokenTag::GT:
	case TokenTag::LTE:
	case TokenTag::GTE:
	case TokenTag::EQUAL:
	case TokenTag::NOT_EQUAL: // fallthrough
		return {30, 31};
	case TokenTag::ADD:
	case TokenTag::SUB: // fallthrough
		return {40, 41};
	case TokenTag::MUL:
	case TokenTag::DIV: // fallthrough
		return {50, 51};
	case TokenTag::PAREN_OPEN:   // a bit of a hack
	case TokenTag::BRACKET_OPEN: // a bit of a hack
	case TokenTag::POLY_OPEN:    // a bit of a hack
	case TokenTag::BRACE_OPEN:   // a bit of a hack
	case TokenTag::DOT:
		return {70, 71};
	default:
		assert(false);
	}
}

Writer<std::vector<AST::AST*>> Parser::parse_argument_list() {
	Writer<std::vector<AST::AST*>> result = {
	    {"Parse Error: Failed to argument list"}};

	if (handle_error(result, require(TokenTag::PAREN_OPEN))) {
		return result;
	}

	auto args =
	    parse_expression_list(TokenTag::COMMA, TokenTag::PAREN_CLOSE, false);

	if (handle_error(result, args)) {
		return result;
	}

	return args;
}

/* The algorithm used here is called 'Pratt Parsing'
 * Here is an article with more information:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
Writer<AST::AST*> Parser::parse_expression(int bp) {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse expression"}};

	Writer<AST::AST*> lhs;

	lhs = parse_terminal();
	if (handle_error(result, lhs)) {
		return result;
	}

	while (1) {
		auto op = peek();

		if (op->m_type == TokenTag::SEMICOLON ||
		    op->m_type == TokenTag::END ||
		    op->m_type == TokenTag::BRACE_CLOSE ||
		    op->m_type == TokenTag::BRACKET_CLOSE ||
		    op->m_type == TokenTag::PAREN_CLOSE ||
		    op->m_type == TokenTag::COMMA ||
		    op->m_type == TokenTag::KEYWORD_THEN ||
		    op->m_type == TokenTag::KEYWORD_ELSE) {
			break;
		}

		if (not is_binary_operator(op->m_type)) {
			result.m_error.m_sub_errors.push_back(
			    make_expected_error("a binary operator", op));
			return result;
		}

		auto op_bp = binding_power_of(op->m_type);
		auto& lp = op_bp.left;
		auto& rp = op_bp.right;

		if (lp < bp)
			break;

		if (op->m_type == TokenTag::PAREN_OPEN) {
			auto args = parse_argument_list();
			if (handle_error(result, args)) {
				return result;
			}

			auto e = m_ast_allocator->make<AST::CallExpression>();

			e->m_callee = lhs.m_result;
			e->m_args = std::move(args.m_result);
			lhs.m_result = e;

			continue;
		}

		if (op->m_type == TokenTag::BRACKET_OPEN) {
			m_lexer->advance();

			auto index = parse_expression();
			if (handle_error(result, index)) {
				return result;
			}

			if (handle_error(result, require(TokenTag::BRACKET_CLOSE))) {
				return result;
			}

			auto e = m_ast_allocator->make<AST::IndexExpression>();

			e->m_callee = lhs.m_result;
			e->m_index = index.m_result;
			lhs.m_result = e;

			continue;
		}

		if (match(TokenTag::POLY_OPEN)) {
			auto args = parse_type_term_arguments();
			if (handle_error(result, args))
				return result;

			auto e = m_ast_allocator->make<AST::TypeTerm>();

			e->m_callee = lhs.m_result;
			e->m_args = std::move(args.m_result);
			lhs.m_result = e;

			continue;
		}

		if (consume(TokenTag::DOT)) {
			auto member = require(TokenTag::IDENTIFIER);

			if (handle_error(result, member))
				return result;

			auto e = m_ast_allocator->make<AST::AccessExpression>();
			e->m_record = lhs.m_result;
			e->m_member = member.m_result;
			lhs.m_result = e;

			continue;
		}

		if (consume(TokenTag::BRACE_OPEN)) {
			auto args =
			    parse_expression_list(TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true);

			if (handle_error(result, args))
				return result;

			auto e = m_ast_allocator->make<AST::ConstructorExpression>();
			e->m_constructor = lhs.m_result;
			e->m_args = std::move(args.m_result);
			lhs.m_result = e;

			continue;
		}

		m_lexer->advance();
		auto rhs = parse_expression(rp);

		auto e = m_ast_allocator->make<AST::BinaryExpression>();

		e->m_op_token = op;
		e->m_lhs = lhs.m_result;
		e->m_rhs = rhs.m_result;

		lhs.m_result = e;
	}

	return lhs;
}

/* We say a terminal is any expression that is not an infix expression.
 * This is not what the term usually means in the literature.
 */
Writer<AST::AST*> Parser::parse_terminal() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse terminal expression"}};

	auto token = peek();

	if (token->m_type == TokenTag::KEYWORD_NULL) {
		auto e = m_ast_allocator->make<AST::NullLiteral>();
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::KEYWORD_TRUE) {
		auto e = m_ast_allocator->make<AST::BooleanLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::KEYWORD_FALSE) {
		auto e = m_ast_allocator->make<AST::BooleanLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::INTEGER) {
		auto e = m_ast_allocator->make<AST::IntegerLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::NUMBER) {
		auto e = m_ast_allocator->make<AST::NumberLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::IDENTIFIER) {
		auto e = m_ast_allocator->make<AST::Identifier>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::STRING) {
		auto e = m_ast_allocator->make<AST::StringLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<AST::AST*>(e);
	}

	if (token->m_type == TokenTag::KEYWORD_FN) {
		auto function = parse_function();
		if (handle_error(result, function))
			return result;
		return function;
	}

	if (token->m_type == TokenTag::PAREN_OPEN and
	    peek(1)->m_type == TokenTag::KEYWORD_IF) {
		m_lexer->advance();
		auto ternary = parse_ternary_expression();
		if (handle_error(result, ternary))
			return result;
		if (handle_error(result, require(TokenTag::PAREN_CLOSE)))
			return result;
		return ternary;
	}

	// parse a parenthesized expression.
	if (token->m_type == TokenTag::PAREN_OPEN) {
		m_lexer->advance();
		auto expr = parse_expression();
		if (handle_error(result, expr))
			return result;
		if (handle_error(result, require(TokenTag::PAREN_CLOSE)))
			return result;
		return expr;
	}

	if (token->m_type == TokenTag::KEYWORD_OBJECT) {
		auto object = parse_object_literal();
		if (handle_error(result, object))
			return result;
		return object;
	}

	if (token->m_type == TokenTag::KEYWORD_DICT) {
		auto dictionary = parse_dictionary_literal();
		if (handle_error(result, dictionary))
			return result;
		return dictionary;
	}

	if (token->m_type == TokenTag::KEYWORD_ARRAY) {
		auto array = parse_array_literal();
		if (handle_error(result, array))
			return result;
		return array;
	}

	if (token->m_type == TokenTag::KEYWORD_UNION ||
	    token->m_type == TokenTag::KEYWORD_STRUCT) {
		// TODO: do the other type functions
		auto type = parse_type_function();
		if (handle_error(result, type))
			return result;
		return type;
	}

	result.m_error.m_sub_errors.push_back({make_expected_error(
	    token_string[int(TokenTag::KEYWORD_FN)], token)});

	result.m_error.m_sub_errors.push_back({make_expected_error(
	    token_string[int(TokenTag::IDENTIFIER)], token)});

	result.m_error.m_sub_errors.push_back(
	    {make_expected_error(token_string[int(TokenTag::NUMBER)], token)});

	return result;
}

Writer<AST::AST*> Parser::parse_ternary_expression() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse ternary expression"}};

	if (handle_error(result, require(TokenTag::KEYWORD_IF)))
		return result;

	auto condition = parse_expression();
	if (handle_error(result, condition))
		return result;

	if (handle_error(result, require(TokenTag::KEYWORD_THEN)))
		return result;

	auto then_expr = parse_expression();
	if (handle_error(result, then_expr))
		return result;

	if (handle_error(result, require(TokenTag::KEYWORD_ELSE)))
		return result;

	auto else_expr = parse_expression();
	if (handle_error(result, else_expr))
		return result;

	auto e = m_ast_allocator->make<AST::TernaryExpression>();

	e->m_condition = condition.m_result;
	e->m_then_expr = then_expr.m_result;
	e->m_else_expr = else_expr.m_result;

	return make_writer<AST::AST*>(e);
}

Writer<AST::Identifier*> Parser::parse_identifier(bool types_allowed) {
	Writer<AST::Identifier*> result = {
	    {"Parse Error: Failed to parse identifier"}};

	Token const* token;

	if (types_allowed and
	   (match(TokenTag::KEYWORD_DICT) or
	    match(TokenTag::KEYWORD_OBJECT) or
	    match(TokenTag::KEYWORD_ARRAY))) {
		token = peek();
		m_lexer->advance();
	} else {
		auto identifier = require(TokenTag::IDENTIFIER);
		if (handle_error(result, identifier))
			return result;
		token = identifier.m_result;
	}

	auto e = m_ast_allocator->make<AST::Identifier>();
	e->m_token = token;
	return make_writer<AST::Identifier*>(e);
}

Writer<AST::AST*> Parser::parse_array_literal() {
	Writer<AST::AST*> result = {
	    {"Failed to parse array literal"}};

	if (handle_error(result, require(TokenTag::KEYWORD_ARRAY))) {
		return result;
	}

	if (handle_error(result, require(TokenTag::BRACE_OPEN))) {
		return result;
	}

	auto elements = parse_expression_list(
	    TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true);

	if (handle_error(result, elements)) {
		return result;
	}

	auto e = m_ast_allocator->make<AST::ArrayLiteral>();
	e->m_elements = std::move(elements.m_result);

	return make_writer<AST::AST*>(e);
}

Writer<AST::AST*> Parser::parse_object_literal() {
	Writer<AST::AST*> result = {
	    {"Failed to parse object literal"}};

	if (handle_error(result, require(TokenTag::KEYWORD_OBJECT))) {
		return result;
	}

	if (handle_error(result, require(TokenTag::BRACE_OPEN))) {
		return result;
	}

	auto declarations = parse_declaration_list(TokenTag::BRACE_CLOSE);

	if (handle_error(result, declarations)) {
		return result;
	}

	if (handle_error(result, require(TokenTag::BRACE_CLOSE))) {
		return result;
	}

	auto e = m_ast_allocator->make<AST::ObjectLiteral>();
	e->m_body = std::move(declarations.m_result);

	return make_writer<AST::AST*>(e);
}

Writer<AST::AST*> Parser::parse_dictionary_literal() {
	Writer<AST::AST*> result = {
	    {"Failed to parse dictionary literal"}};

	if (handle_error(result, require(TokenTag::KEYWORD_DICT))) {
		return result;
	}

	if (handle_error(result, require(TokenTag::BRACE_OPEN))) {
		return result;
	}

	auto declarations = parse_declaration_list(TokenTag::BRACE_CLOSE);

	if (handle_error(result, declarations)) {
		return result;
	}

	if (handle_error(result, require(TokenTag::BRACE_CLOSE))) {
		return result;
	}

	auto e = m_ast_allocator->make<AST::DictionaryLiteral>();
	e->m_body = std::move(declarations.m_result);

	return make_writer<AST::AST*>(e);
}

/*
 * functions look like this:
 * fn (x : int, y, z : string) {
 *   print(x);
 * }
 */
Writer<AST::AST*> Parser::parse_function() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse function"}};

	if (handle_error(result, require(TokenTag::KEYWORD_FN)))
		return result;

	if (handle_error(result, require(TokenTag::PAREN_OPEN)))
		return result;

	std::vector<AST::Declaration> args;
	while (1) {
		if (consume(TokenTag::PAREN_CLOSE)) {
			break;
		} else if (match(TokenTag::IDENTIFIER)) {
			// consume argument name

			AST::Declaration arg;

			arg.m_identifier_token = peek();
			m_lexer->advance();

			if (consume(TokenTag::DECLARE)) {
				// optionally consume a type hint
				auto type = parse_type_term();
				if (handle_error(result, type))
					return result;
				arg.m_type_hint = type.m_result;
			}

			args.push_back(std::move(arg));

			if (consume(TokenTag::COMMA)) {
				// If we find a comma, we have to parse
				// another argument, so we loop again.
				continue;
			} else if (consume(TokenTag::PAREN_CLOSE)) {
				// If we find a closing paren, we are done
				// parsing arguments, so we stop.
				break;
			} else {
				// Anything else is unexpected input, so we
				// report an error.
				result.m_error.m_sub_errors.push_back(
				    make_expected_error("',' or ')'", peek()));
				return result;
			}
		} else {
			result.m_error.m_sub_errors.push_back(
			    make_expected_error("an argument name (IDENTIFIER)", peek()));
			return result;
		}
	}

	if (consume(TokenTag::ARROW)) {
		auto expression = parse_expression();
		if (handle_error(result, expression))
			return result;

		auto e = m_ast_allocator->make<AST::ShortFunctionLiteral>();
		e->m_body = expression.m_result;
		e->m_args = std::move(args);

		return make_writer<AST::AST*>(e);
	} else {
		auto block = parse_block();
		if (handle_error(result, block))
			return result;

		auto e = m_ast_allocator->make<AST::FunctionLiteral>();
		e->m_body = block.m_result;
		e->m_args = std::move(args);

		return make_writer<AST::AST*>(e);
	}
}

Writer<AST::AST*> Parser::parse_block() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse block statement"}};

	if (handle_error(result, require(TokenTag::BRACE_OPEN))) {
		return result;
	}

	std::vector<AST::AST*> statements;

	// loop until we find a matching closing bracket
	while (1) {
		auto p0 = peek();

		if (p0->m_type == TokenTag::END) {
			result.m_error.m_sub_errors.push_back(
			    {{"Found EOF while parsing block statement"}});
			return result;
		}

		if (p0->m_type == TokenTag::BRACE_CLOSE) {
			m_lexer->advance();
			break;
		}

		auto statement = parse_statement();
		if (handle_error(result, statement)) {
			return result;
		} else {
			statements.push_back(statement.m_result);
		}
	}

	auto e = m_ast_allocator->make<AST::Block>();

	e->m_body = std::move(statements);

	return make_writer<AST::AST*>(e);
}

Writer<AST::AST*> Parser::parse_return_statement() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse return statement"}};

	if (handle_error(result, require(TokenTag::KEYWORD_RETURN))) {
		return result;
	}

	auto value = parse_expression();

	if (handle_error(result, value)) {
		return result;
	}

	if (handle_error(result, require(TokenTag::SEMICOLON))) {
		return result;
	}

	auto e = m_ast_allocator->make<AST::ReturnStatement>();

	e->m_value = value.m_result;

	return make_writer<AST::AST*>(e);
}

Writer<AST::AST*> Parser::parse_if_else_statement() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse if-else statement"}};

	if (handle_error(result, require(TokenTag::KEYWORD_IF))) {
		return result;
	}

	if (handle_error(result, require(TokenTag::PAREN_OPEN))) {
		return result;
	}

	auto condition = parse_expression();
	if (handle_error(result, condition)) {
		return result;
	}

	if (handle_error(result, require(TokenTag::PAREN_CLOSE))) {
		return result;
	}

	auto body = parse_statement();
	if (handle_error(result, body)) {
		return result;
	}

	auto e = m_ast_allocator->make<AST::IfElseStatement>();

	e->m_condition = condition.m_result;
	e->m_body = body.m_result;

	if (consume(TokenTag::KEYWORD_ELSE)) {
		auto else_body = parse_statement();

		if (handle_error(result, else_body))
			return result;

		e->m_else_body = else_body.m_result;
	}

	return make_writer<AST::AST*>(e);
}

Writer<AST::AST*> Parser::parse_for_statement() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse for statement"}};

	if (handle_error(result, require(TokenTag::KEYWORD_FOR))) {
		return result;
	}

	if (handle_error(result, require(TokenTag::PAREN_OPEN))) {
		return result;
	}

	// NOTE: handles semicolon already
	auto declaration = parse_declaration();
	if (handle_error(result, declaration)) {
		return result;
	}

	auto condition = parse_expression();
	if (handle_error(result, condition)) {
		return result;
	}
	if (handle_error(result, require(TokenTag::SEMICOLON))) {
		return result;
	}

	auto action = parse_expression();
	if (handle_error(result, action)) {
		return result;
	}

	if (handle_error(result, require(TokenTag::PAREN_CLOSE))) {
		return result;
	}

	auto body = parse_statement();
	if (handle_error(result, body)) {
		return result;
	}

	auto e = m_ast_allocator->make<AST::ForStatement>();

	e->m_declaration = std::move(*declaration.m_result);
	e->m_condition = condition.m_result;
	e->m_action = action.m_result;
	e->m_body = body.m_result;

	return make_writer<AST::AST*>(e);
}

Writer<AST::AST*> Parser::parse_while_statement() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse while statement"}};

	if (handle_error(result, require(TokenTag::KEYWORD_WHILE)))
		return result;

	if (handle_error(result, require(TokenTag::PAREN_OPEN)))
		return result;

	auto condition = parse_expression();
	if (handle_error(result, condition))
		return result;

	if (handle_error(result, require(TokenTag::PAREN_CLOSE)))
		return result;

	auto body = parse_statement();
	if (handle_error(result, body))
		return result;

	auto e = m_ast_allocator->make<AST::WhileStatement>();

	e->m_condition = condition.m_result;
	e->m_body = body.m_result;

	return make_writer<AST::AST*>(e);
}

/*
 * Looks ahead a few tokens to predict what syntactic structure we are about to
 * parse. This prevents us from backtracking and ensures the parser runs in
 * linear time.
 */
Writer<AST::AST*> Parser::parse_statement() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse statement"}};

	// TODO: paren_open, string tokens, integer and numer
	// tokens, etc should also be recognized as expressions.
	auto* p0 = peek(0);
	if (p0->m_type == TokenTag::IDENTIFIER) {
		auto* p1 = peek(1);

		if (p1->m_type == TokenTag::DECLARE ||
		    p1->m_type == TokenTag::DECLARE_ASSIGN) {

			auto declaration = parse_declaration();

			if (handle_error(result, declaration)) {
				return result;
			}

			return make_writer<AST::AST*>(declaration.m_result);
		} else {

			// TODO: wrap in an ExpressionStatement ?

			auto expression = parse_expression();

			if (handle_error(result, expression)) {
				return result;
			}

			if (handle_error(result, require(TokenTag::SEMICOLON))) {
				return result;
			}

			return expression;
		}
	} else if (p0->m_type == TokenTag::KEYWORD_RETURN) {
		auto return_statement = parse_return_statement();
		if (handle_error(result, return_statement)) {
			return result;
		}
		return return_statement;
	} else if (p0->m_type == TokenTag::KEYWORD_IF) {
		auto if_else_statement = parse_if_else_statement();
		if (handle_error(result, if_else_statement)) {
			return result;
		}
		return if_else_statement;
	} else if (p0->m_type == TokenTag::KEYWORD_FOR) {
		auto for_statement = parse_for_statement();
		if (handle_error(result, for_statement)) {
			return result;
		}
		return for_statement;
	} else if (p0->m_type == TokenTag::KEYWORD_WHILE) {
		auto while_statement = parse_while_statement();
		if (handle_error(result, while_statement))
			return result;
		return while_statement;
	} else if (p0->m_type == TokenTag::BRACE_OPEN) {
		auto block_statement = parse_block();
		if (handle_error(result, block_statement)) {
			return result;
		}
		return block_statement;
	} else {
		auto err = make_expected_error("a statement", p0);

		result.m_error.m_sub_errors.push_back(std::move(err));
		return result;
	}

	return result;
}

Writer<std::vector<AST::AST*>> Parser::parse_type_term_arguments() {
	Writer<std::vector<AST::AST*>> result = {
	    {"Parse Error: Failed to parse type arguments"}};

	if (handle_error(result, require(TokenTag::POLY_OPEN)))
		return result;

	std::vector<AST::AST*> args;

	while (!consume(TokenTag::POLY_CLOSE)) {
		auto arg = parse_type_term();
		if (handle_error(result, arg))
			return result;

		args.push_back(arg.m_result);
	}

	return make_writer(std::move(args));
}

Writer<AST::AST*> Parser::parse_type_term() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse type"}};

	auto callee = parse_identifier(true);
	if (handle_error(result, callee))
		return result;

	auto e = m_ast_allocator->make<AST::TypeTerm>();
	e->m_callee = callee.m_result;

	if (!match(TokenTag::POLY_OPEN))
		return make_writer<AST::AST*>(e);

	auto args = parse_type_term_arguments();
	if (handle_error(result, args))
		return result;

	e->m_args = std::move(args.m_result);
	return make_writer<AST::AST*>(e);
}

Writer<std::pair<std::vector<AST::Identifier>, std::vector<AST::AST*>>> Parser::parse_type_list(
    bool with_identifiers = false) {
	Writer<std::pair<std::vector<AST::Identifier>, std::vector<AST::AST*>>>
	    result = {{"Parse Error: Failed to parse type list"}};

	std::vector<AST::Identifier> identifiers;
	std::vector<AST::AST*> types;

	if (handle_error(result, require(TokenTag::BRACE_OPEN)))
		return result;

	while(!consume(TokenTag::BRACE_CLOSE)) {
		if (with_identifiers) {
			auto cons = parse_identifier();
			if (handle_error(result, cons))
				return result;

			if (handle_error(result, require(TokenTag::DECLARE)))
				return result;

			identifiers.push_back(std::move(*cons.m_result));
		}

		auto type = parse_type_term();
		if (handle_error(result, type))
			return result;

		if (handle_error(result, require(TokenTag::SEMICOLON)))
			return result;

		types.push_back(type.m_result);
	}

	return make_writer<
	    std::pair<std::vector<AST::Identifier>, std::vector<AST::AST*>>>(
	    make_pair(std::move(identifiers), std::move(types)));
}

Writer<AST::AST*> Parser::parse_type_var() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse type var"}};

	if (handle_error(result, require(TokenTag::AT)))
		return result;

	auto token = require(TokenTag::IDENTIFIER);
	if (handle_error(result, token))
		return result;

	auto t = m_ast_allocator->make<AST::TypeVar>();
	t->m_token = token.m_result;

	return make_writer<AST::AST*>(t);
}

Writer<AST::AST*> Parser::parse_type_function() {
	Writer<AST::AST*> result = {
	    {"Parse Error: Failed to parse type function"}};

	if (consume(TokenTag::KEYWORD_UNION)) {
		auto tl = parse_type_list(true);
		if (handle_error(result, tl))
			return result;

		auto u = m_ast_allocator->make<AST::UnionExpression>();
		u->m_constructors = std::move(tl.m_result.first);
		u->m_types = std::move(tl.m_result.second);

		return make_writer<AST::AST*>(u);
	} else if (consume(TokenTag::KEYWORD_TUPLE)) {
		auto tl = parse_type_list(false);
		if (handle_error(result, tl))
			return result;

		auto t = m_ast_allocator->make<AST::TupleExpression>();
		t->m_types = std::move(tl.m_result.second);

		return make_writer<AST::AST*>(t);
	} else if (consume(TokenTag::KEYWORD_STRUCT)) {
		auto tl = parse_type_list(true);
		if (handle_error(result, tl))
			return result;

		auto s = m_ast_allocator->make<AST::StructExpression>();
		s->m_fields = std::move(tl.m_result.first);
		s->m_types = std::move(tl.m_result.second);

		return make_writer<AST::AST*>(s);
	}

	return result;
}
