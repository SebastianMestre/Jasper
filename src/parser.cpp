#include "parser.hpp"

#include "string_view.hpp"
#include <sstream>

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
	   << token_type_string[int(found_token->m_type)] << ' '
	   << found_token->m_text << " instead";

	return ErrorReport {ss.str()};
}

Writer<Token const*> Parser::require(token_type expected_type) {
	Token const* current_token = &m_lexer->current_token();

	if (current_token->m_type != expected_type) {
		return {make_expected_error(
		    token_type_string[int(expected_type)], current_token)};
	}

	m_lexer->advance();

	return make_writer(current_token);
}

bool Parser::consume(token_type expected_type) {
	if (match(expected_type)) {
		m_lexer->advance();
		return true;
	}
	return false;
}

bool Parser::match(token_type expected_type) {
	Token const* current_token = &m_lexer->current_token();
	if (current_token->m_type != expected_type)
		return false;
	return true;
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_top_level() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse top level program"}};

	auto declarations = parse_declaration_list(token_type::END);

	if (handle_error(result, declarations)) {
		return result;
	}

	auto e = std::make_unique<AST::DeclarationList>();

	e->m_declarations = std::move(declarations.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::vector<std::unique_ptr<AST::AST>>> Parser::parse_declaration_list(
    token_type terminator) {
	Writer<std::vector<std::unique_ptr<AST::AST>>> result = {
	    {"Parse Error: Failed to parse declaration list"}};

	std::vector<std::unique_ptr<AST::AST>> declarations;

	while (1) {
		auto p0 = peek();

		if (p0->m_type == terminator)
			break;

		if (p0->m_type == token_type::END) {
			result.m_error.m_sub_errors.push_back(
			    make_expected_error("a declaration", p0));

			result.m_error.m_sub_errors.push_back(
			    make_expected_error(token_type_string[int(terminator)], p0));

			return result;
		}

		auto declaration = parse_declaration();

		if (handle_error(result, declaration))
			return result;

		declarations.push_back(std::move(declaration.m_result));
	}

	return make_writer(std::move(declarations));
}

Writer<std::vector<std::unique_ptr<AST::AST>>> Parser::parse_expression_list(
    token_type delimiter, token_type terminator, bool allow_trailing_delimiter) {
	Writer<std::vector<std::unique_ptr<AST::AST>>> result = {
	    {"Parse Error: Failed to parse expression list"}};

	std::vector<std::unique_ptr<AST::AST>> expressions;

	if (peek()->m_type == terminator) {
		m_lexer->advance();
	} else {
		while (1) {
			auto p0 = peek();

			if (p0->m_type == token_type::END) {
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
				expressions.push_back(std::move(expression.m_result));
			}

			auto p1 = peek();

			if (p1->m_type == delimiter) {
				m_lexer->advance();
			} else if (p1->m_type == terminator) {
				m_lexer->advance();
				break;
			} else {
				result.m_error.m_sub_errors.push_back(
				    make_expected_error(token_type_string[(int)delimiter], p1));
				result.m_error.m_sub_errors.push_back(
				    make_expected_error(token_type_string[(int)terminator], p1));
				return result;
			}
		}
	}

	return make_writer(std::move(expressions));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_declaration() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse declaration"}};

	Writer<Token const*> name = require(token_type::IDENTIFIER);

	if (handle_error(result, name))
		return result;

	Writer<std::unique_ptr<AST::AST>> type;

	if (consume(token_type::DECLARE_ASSIGN)) {
	} else if (consume(token_type::DECLARE)) {

		type = parse_type_term();
		if (handle_error(result, type))
			return result;

		if (handle_error(result, require(token_type::ASSIGN)))
			return result;
	} else {
		result.m_error.m_sub_errors.push_back(
		    make_expected_error("':' or ':='", peek()));
	}

	auto value = parse_expression();

	if (handle_error(result, value))
		return result;

	if (handle_error(result, require(token_type::SEMICOLON)))
		return result;

	auto p = std::make_unique<AST::Declaration>();

	p->m_identifier_token = name.m_result;
	p->m_type = std::move(type.m_result);
	p->m_value = std::move(value.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(p));
}

// These are important for infix expression parsing.
// TODO: clean up

struct binding_power {
	int left, right;
};

bool is_binary_operator(token_type t) {
	// TODO: fill out this table
	switch (t) {
	case token_type::PAREN_OPEN:   // a bit of a hack
	case token_type::BRACKET_OPEN: // a bit of a hack

	case token_type::LT:
	case token_type::GT:
	case token_type::LTE:
	case token_type::GTE:
	case token_type::EQUAL:
	case token_type::NOT_EQUAL:
	case token_type::ASSIGN:
	case token_type::DOT:
	case token_type::PIZZA:
	case token_type::ADD:
	case token_type::SUB:
	case token_type::MUL:
	case token_type::DIV: // fallthrough
		return true;
	default:
		return false;
	}
}

binding_power binding_power_of(token_type t) {
	// TODO: fill out this table
	switch (t) {

	case token_type::ASSIGN:
		return {10, 11};
	case token_type::PIZZA:
		return {20, 21};
	case token_type::LT:
	case token_type::GT:
	case token_type::LTE:
	case token_type::GTE:
	case token_type::EQUAL:
	case token_type::NOT_EQUAL: // fallthrough
		return {30, 31};
	case token_type::ADD:
	case token_type::SUB: // fallthrough
		return {40, 41};
	case token_type::MUL:
	case token_type::DIV: // fallthrough
		return {50, 51};
	case token_type::PAREN_OPEN:   // a bit of a hack
	case token_type::BRACKET_OPEN: // a bit of a hack
	case token_type::DOT:
		return {70, 71};
	default:
		assert(false);
	}
}

Writer<std::vector<std::unique_ptr<AST::AST>>> Parser::parse_argument_list() {
	Writer<std::vector<std::unique_ptr<AST::AST>>> result = {
	    {"Parse Error: Failed to argument list"}};

	if (handle_error(result, require(token_type::PAREN_OPEN))) {
		return result;
	}

	auto args =
	    parse_expression_list(token_type::COMMA, token_type::PAREN_CLOSE, false);

	if (handle_error(result, args)) {
		return result;
	}

	return args;
}

/* The algorithm used here is called 'Pratt Parsing'
 * Here is an article with more information:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
Writer<std::unique_ptr<AST::AST>> Parser::parse_expression(int bp) {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse expression"}};

	Writer<std::unique_ptr<AST::AST>> lhs;

	lhs = parse_terminal();
	if (handle_error(result, lhs)) {
		return result;
	}

	while (1) {
		auto op = peek();

		if (op->m_type == token_type::SEMICOLON || op->m_type == token_type::END ||
		    op->m_type == token_type::BRACE_CLOSE ||
		    op->m_type == token_type::BRACKET_CLOSE ||
		    op->m_type == token_type::PAREN_CLOSE ||
		    op->m_type == token_type::COMMA ||
		    op->m_type == token_type::KEYWORD_THEN ||
		    op->m_type == token_type::KEYWORD_ELSE) {
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

		if (op->m_type == token_type::PAREN_OPEN) {
			auto args = parse_argument_list();
			if (handle_error(result, args)) {
				return result;
			}

			auto e = std::make_unique<AST::CallExpression>();

			e->m_callee = std::move(lhs.m_result);
			e->m_args = std::move(args.m_result);
			lhs.m_result = std::move(e);

			continue;
		}

		if (op->m_type == token_type::BRACKET_OPEN) {
			m_lexer->advance();

			auto index = parse_expression();
			if (handle_error(result, index)) {
				return result;
			}

			if (handle_error(result, require(token_type::BRACKET_CLOSE))) {
				return result;
			}

			auto e = std::make_unique<AST::IndexExpression>();
			e->m_callee = std::move(lhs.m_result);
			e->m_index = std::move(index.m_result);
			lhs.m_result = std::move(e);

			continue;
		}

		m_lexer->advance();
		auto rhs = parse_expression(rp);

		auto e = std::make_unique<AST::BinaryExpression>();

		e->m_op_token = op;
		e->m_lhs = std::move(lhs.m_result);
		e->m_rhs = std::move(rhs.m_result);

		lhs.m_result = std::move(e);
	}

	return lhs;
}

/* We say a terminal is any expression that is not an infix expression.
 * This is not what the term usually means in the literature.
 */
Writer<std::unique_ptr<AST::AST>> Parser::parse_terminal() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse terminal expression"}};

	auto token = peek();

	if (token->m_type == token_type::KEYWORD_NULL) {
		auto e = std::make_unique<AST::NullLiteral>();
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::KEYWORD_TRUE) {
		auto e = std::make_unique<AST::BooleanLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::KEYWORD_FALSE) {
		auto e = std::make_unique<AST::BooleanLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::INTEGER) {
		auto e = std::make_unique<AST::IntegerLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::NUMBER) {
		auto e = std::make_unique<AST::NumberLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::IDENTIFIER) {
		auto e = std::make_unique<AST::Identifier>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::STRING) {
		auto e = std::make_unique<AST::StringLiteral>();
		e->m_token = token;
		m_lexer->advance();
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}

	if (token->m_type == token_type::KEYWORD_FN) {
		auto function = parse_function();
		if (handle_error(result, function))
			return result;
		return function;
	}

	if (token->m_type == token_type::PAREN_OPEN and
	    peek(1)->m_type == token_type::KEYWORD_IF) {
		m_lexer->advance();
		auto ternary = parse_ternary_expression();
		if (handle_error(result, ternary))
			return result;
		if (handle_error(result, require(token_type::PAREN_CLOSE)))
			return result;
		return ternary;
	}

	// parse a parenthesized expression.
	if (token->m_type == token_type::PAREN_OPEN) {
		m_lexer->advance();
		auto expr = parse_expression();
		if (handle_error(result, expr))
			return result;
		if (handle_error(result, require(token_type::PAREN_CLOSE)))
			return result;
		return expr;
	}

	if (token->m_type == token_type::KEYWORD_OBJECT) {
		auto object = parse_object_literal();
		if (handle_error(result, object))
			return result;
		return object;
	}

	if (token->m_type == token_type::KEYWORD_DICT) {
		auto dictionary = parse_dictionary_literal();
		if (handle_error(result, dictionary))
			return result;
		return dictionary;
	}

	if (token->m_type == token_type::KEYWORD_ARRAY) {
		auto array = parse_array_literal();
		if (handle_error(result, array))
			return result;
		return array;
	}

	result.m_error.m_sub_errors.push_back({make_expected_error(
	    token_type_string[int(token_type::KEYWORD_FN)], token)});

	result.m_error.m_sub_errors.push_back({make_expected_error(
	    token_type_string[int(token_type::IDENTIFIER)], token)});

	result.m_error.m_sub_errors.push_back(
	    {make_expected_error(token_type_string[int(token_type::NUMBER)], token)});

	return result;
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_ternary_expression() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse ternary expression"}};

	if (handle_error(result, require(token_type::KEYWORD_IF)))
		return result;

	auto condition = parse_expression();
	if (handle_error(result, condition))
		return result;

	if (handle_error(result, require(token_type::KEYWORD_THEN)))
		return result;

	auto then_expr = parse_expression();
	if (handle_error(result, then_expr))
		return result;

	if (handle_error(result, require(token_type::KEYWORD_ELSE)))
		return result;

	auto else_expr = parse_expression();
	if (handle_error(result, else_expr))
		return result;

	auto e = std::make_unique<AST::TernaryExpression>();

	e->m_condition = std::move(condition.m_result);
	e->m_then_expr = std::move(then_expr.m_result);
	e->m_else_expr = std::move(else_expr.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_identifier() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse identifier"}};

	auto token = require(token_type::IDENTIFIER);
	if (handle_error(result, token))
		return result;

	auto e = std::make_unique<AST::Identifier>();
	e->m_token = token.m_result;
	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_array_literal() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Failed to parse array literal"}};

	if (handle_error(result, require(token_type::KEYWORD_ARRAY))) {
		return result;
	}

	if (handle_error(result, require(token_type::BRACE_OPEN))) {
		return result;
	}

	auto elements = parse_expression_list(
	    token_type::SEMICOLON, token_type::BRACE_CLOSE, true);

	if (handle_error(result, elements)) {
		return result;
	}

	auto e = std::make_unique<AST::ArrayLiteral>();
	e->m_elements = std::move(elements.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_object_literal() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Failed to parse object literal"}};

	if (handle_error(result, require(token_type::KEYWORD_OBJECT))) {
		return result;
	}

	if (handle_error(result, require(token_type::BRACE_OPEN))) {
		return result;
	}

	auto declarations = parse_declaration_list(token_type::BRACE_CLOSE);

	if (handle_error(result, declarations)) {
		return result;
	}

	if (handle_error(result, require(token_type::BRACE_CLOSE))) {
		return result;
	}

	auto e = std::make_unique<AST::ObjectLiteral>();
	e->m_body = std::move(declarations.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_dictionary_literal() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Failed to parse dictionary literal"}};

	if (handle_error(result, require(token_type::KEYWORD_DICT))) {
		return result;
	}

	if (handle_error(result, require(token_type::BRACE_OPEN))) {
		return result;
	}

	auto declarations = parse_declaration_list(token_type::BRACE_CLOSE);

	if (handle_error(result, declarations)) {
		return result;
	}

	if (handle_error(result, require(token_type::BRACE_CLOSE))) {
		return result;
	}

	auto e = std::make_unique<AST::DictionaryLiteral>();
	e->m_body = std::move(declarations.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

/*
 * functions look like this:
 * fn (x : int, y, z : string) {
 *   print(x);
 * }
 */
Writer<std::unique_ptr<AST::AST>> Parser::parse_function() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse function"}};

	if (handle_error(result, require(token_type::KEYWORD_FN)))
		return result;

	if (handle_error(result, require(token_type::PAREN_OPEN)))
		return result;

	std::vector<std::unique_ptr<AST::AST>> args;
	while (1) {
		if (consume(token_type::PAREN_CLOSE)) {
			break;
		} else if (match(token_type::IDENTIFIER)) {
			// consume argument name

			auto arg = std::make_unique<AST::Declaration>();

			arg->m_identifier_token = peek();
			m_lexer->advance();

			if (consume(token_type::DECLARE)) {
				// optionally consume a type hint
				auto type = parse_type_term();
				if (handle_error(result, type))
					return result;
				arg->m_type = std::move(type.m_result);
			}

			args.push_back(std::move(arg));

			if (consume(token_type::COMMA)) {
				// If we find a comma, we have to parse
				// another argument, so we loop again.
				continue;
			} else if (consume(token_type::PAREN_CLOSE)) {
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

	if (consume(token_type::ARROW)) {
		auto expression = parse_expression();
		if (handle_error(result, expression))
			return result;

		auto e = std::make_unique<AST::ShortFunctionLiteral>();
		e->m_body = std::move(expression.m_result);
		e->m_args = std::move(args);

		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	} else {
		auto block = parse_block();
		if (handle_error(result, block))
			return result;

		auto e = std::make_unique<AST::FunctionLiteral>();
		e->m_body = std::move(block.m_result);
		e->m_args = std::move(args);

		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
	}
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_block() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse block statement"}};

	if (handle_error(result, require(token_type::BRACE_OPEN))) {
		return result;
	}

	std::vector<std::unique_ptr<AST::AST>> statements;

	// loop until we find a matching closing bracket
	while (1) {
		auto p0 = peek();

		if (p0->m_type == token_type::END) {
			result.m_error.m_sub_errors.push_back(
			    {{"Found EOF while parsing block statement"}});
			return result;
		}

		if (p0->m_type == token_type::BRACE_CLOSE) {
			m_lexer->advance();
			break;
		}

		auto statement = parse_statement();
		if (handle_error(result, statement)) {
			return result;
		} else {
			statements.push_back(std::move(statement.m_result));
		}
	}

	auto e = std::make_unique<AST::Block>();

	e->m_body = std::move(statements);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_return_statement() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse return statement"}};

	if (handle_error(result, require(token_type::KEYWORD_RETURN))) {
		return result;
	}

	auto value = parse_expression();

	if (handle_error(result, value)) {
		return result;
	}

	if (handle_error(result, require(token_type::SEMICOLON))) {
		return result;
	}

	auto e = std::make_unique<AST::ReturnStatement>();

	e->m_value = std::move(value.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_if_else_statement() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse if-else statement"}};

	if (handle_error(result, require(token_type::KEYWORD_IF))) {
		return result;
	}

	if (handle_error(result, require(token_type::PAREN_OPEN))) {
		return result;
	}

	auto condition = parse_expression();
	if (handle_error(result, condition)) {
		return result;
	}

	if (handle_error(result, require(token_type::PAREN_CLOSE))) {
		return result;
	}

	auto body = parse_statement();
	if (handle_error(result, body)) {
		return result;
	}

	auto e = std::make_unique<AST::IfElseStatement>();

	e->m_condition = std::move(condition.m_result);
	e->m_body = std::move(body.m_result);

	if (consume(token_type::KEYWORD_ELSE)) {
		auto else_body = parse_statement();

		if (handle_error(result, else_body))
			return result;

		e->m_else_body = std::move(else_body.m_result);
	}

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_for_statement() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse for statement"}};

	if (handle_error(result, require(token_type::KEYWORD_FOR))) {
		return result;
	}

	if (handle_error(result, require(token_type::PAREN_OPEN))) {
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
	if (handle_error(result, require(token_type::SEMICOLON))) {
		return result;
	}

	auto action = parse_expression();
	if (handle_error(result, action)) {
		return result;
	}

	if (handle_error(result, require(token_type::PAREN_CLOSE))) {
		return result;
	}

	auto body = parse_statement();
	if (handle_error(result, body)) {
		return result;
	}

	auto e = std::make_unique<AST::ForStatement>();

	e->m_declaration = std::move(declaration.m_result);
	e->m_condition = std::move(condition.m_result);
	e->m_action = std::move(action.m_result);
	e->m_body = std::move(body.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_while_statement() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse while statement"}};

	if (handle_error(result, require(token_type::KEYWORD_WHILE)))
		return result;

	if (handle_error(result, require(token_type::PAREN_OPEN)))
		return result;

	auto condition = parse_expression();
	if (handle_error(result, condition))
		return result;

	if (handle_error(result, require(token_type::PAREN_CLOSE)))
		return result;

	auto body = parse_statement();
	if (handle_error(result, body))
		return result;

	auto e = std::make_unique<AST::WhileStatement>();

	e->m_condition = std::move(condition.m_result);
	e->m_body = std::move(body.m_result);

	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

/*
 * Looks ahead a few tokens to predict what syntactic structure we are about to
 * parse. This prevents us from backtracking and ensures the parser runs in
 * linear time.
 */
Writer<std::unique_ptr<AST::AST>> Parser::parse_statement() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse statement"}};

	// TODO: paren_open, string tokens, integer and numer
	// tokens, etc should also be recognized as expressions.
	auto* p0 = peek(0);
	if (p0->m_type == token_type::IDENTIFIER) {
		auto* p1 = peek(1);

		if (p1->m_type == token_type::DECLARE ||
		    p1->m_type == token_type::DECLARE_ASSIGN) {

			auto declaration = parse_declaration();

			if (handle_error(result, declaration)) {
				return result;
			}

			return declaration;
		} else {

			// TODO: wrap in an ExpressionStatement ?

			auto expression = parse_expression();

			if (handle_error(result, expression)) {
				return result;
			}

			if (handle_error(result, require(token_type::SEMICOLON))) {
				return result;
			}

			return expression;
		}
	} else if (p0->m_type == token_type::KEYWORD_RETURN) {
		auto return_statement = parse_return_statement();
		if (handle_error(result, return_statement)) {
			return result;
		}
		return return_statement;
	} else if (p0->m_type == token_type::KEYWORD_IF) {
		auto if_else_statement = parse_if_else_statement();
		if (handle_error(result, if_else_statement)) {
			return result;
		}
		return if_else_statement;
	} else if (p0->m_type == token_type::KEYWORD_FOR) {
		auto for_statement = parse_for_statement();
		if (handle_error(result, for_statement)) {
			return result;
		}
		return for_statement;
	} else if (p0->m_type == token_type::KEYWORD_WHILE) {
		auto while_statement = parse_while_statement();
		if (handle_error(result, while_statement))
			return result;
		return while_statement;
	} else if (p0->m_type == token_type::BRACE_OPEN) {
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

Writer<std::unique_ptr<AST::AST>> Parser::parse_type_term() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse type"}};

	auto callee = parse_identifier();
	if (handle_error(result, callee))
		return result;

	auto e = std::make_unique<AST::TypeTerm>();
	e->m_callee = std::move(callee.m_result);

	if (!consume(token_type::POLY_OPEN))
		return make_writer<std::unique_ptr<AST::AST>>(std::move(e));

	std::vector<std::unique_ptr<AST::AST>> args;
	while (!consume(token_type::POLY_CLOSE)) {
		auto arg = parse_type_term();
		if (handle_error(result, arg))
			return result;

		args.push_back(std::move(arg.m_result));
	}

	e->m_args = std::move(args);
	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}

Writer<std::unique_ptr<AST::AST>> Parser::parse_union_type_expression() {
	Writer<std::unique_ptr<AST::AST>> result = {
	    {"Parse Error: Failed to parse union"}};

	if (handle_error(result, require(token_type::KEYWORD_UNION)))
		return result;

	if (handle_error(result, require(token_type::BRACE_OPEN)))
		return result;

	if (handle_error(result, require(token_type::BRACE_CLOSE)))
		return result;

	auto e = std::make_unique<AST::UnionTypeExpression>();
	return make_writer<std::unique_ptr<AST::AST>>(std::move(e));
}
