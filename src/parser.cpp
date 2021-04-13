#include "parser.hpp"

#include "utils/string_view.hpp"
#include "cst.hpp"
#include "cst_allocator.hpp"
#include "error_report.hpp"
#include "token_array.hpp"

#include <sstream>
#include <utility>
#include <vector>

#include <cassert>

template <typename T>
Writer<T> make_writer(T x) {
	return {{}, std::move(x)};
}

ErrorReport make_expected_error(string_view expected, Token const* found_token) {
	std::stringstream ss;
	ss << "Parse Error: @ " << found_token->m_line0 + 1 << ":"
	   << found_token->m_col0 + 1 << ": Expected " << expected << " but got "
	   << token_string[int(found_token->m_type)] << ' '
	   << found_token->m_text << " instead";

	return ErrorReport {ss.str()};
}

struct Parser {
	/* token handler */
	TokenArray const& m_tokens;
	CST::Allocator* m_cst_allocator;
	int m_token_cursor { 0 };

	Parser(TokenArray const& tokens, CST::Allocator* cst_allocator)
	    : m_tokens {tokens}
	    , m_cst_allocator {cst_allocator} {}

	Writer<std::vector<CST::Declaration>> parse_declaration_list(TokenTag);
	Writer<std::vector<CST::CST*>> parse_expression_list(TokenTag, TokenTag, bool);

	Writer<CST::CST*> parse_top_level();

	Writer<CST::CST*> parse_sequence_expression();
	Writer<CST::Identifier*> parse_identifier(bool types_allowed = false);
	Writer<CST::Declaration*> parse_declaration();
	Writer<CST::CST*> parse_expression(int bp = 0, CST::CST* parsed_lhs = nullptr);
	Writer<CST::CST*> parse_terminal();
	Writer<CST::CST*> parse_ternary_expression(CST::CST* parsed_condition = nullptr);
	Writer<CST::CST*> parse_function();
	Writer<CST::CST*> parse_array_literal();
	Writer<std::vector<CST::CST*>> parse_argument_list();
	Writer<CST::CST*> parse_block();
	Writer<CST::CST*> parse_statement();
	Writer<CST::CST*> parse_return_statement();
	Writer<CST::CST*> parse_if_else_stmt_or_expr();
	Writer<CST::CST*> parse_for_statement();
	Writer<CST::CST*> parse_while_statement();
	Writer<CST::CST*> parse_match_expression();
	Writer<std::pair<Token const*, CST::CST*>> parse_name_and_type(bool required_type = false);
	Writer<CST::CST*> parse_type_term();
	Writer<std::vector<CST::CST*>> parse_type_term_arguments();
	Writer<std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>> parse_type_list(bool);
	Writer<CST::CST*> parse_type_var();
	Writer<CST::CST*> parse_type_function();

	void advance_token_cursor() {
		m_token_cursor += 1;
	}

	Token const* peek(int dt = 0) {
		int index = m_token_cursor + dt;
		return &m_tokens.at(index);
	}

	Writer<Token const*> require(TokenTag expected_type) {
		Token const* current_token = peek();

		if (current_token->m_type != expected_type) {
			return {make_expected_error(
			    token_string[int(expected_type)], current_token)};
		}

		advance_token_cursor();

		return make_writer(current_token);
	}

	bool match(TokenTag expected_type) {
		Token const* current_token = peek();
		return current_token->m_type == expected_type;
	}

	bool consume(TokenTag expected_type) {
		if (match(expected_type)) {
			advance_token_cursor();
			return true;
		}
		return false;
	}
};

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

#define CHECK_AND_RETURN(result, writer)                                       \
	if (handle_error(result, writer))                                          \
		return result;

#define REQUIRE(result, token)                                                 \
	if (handle_error(result, require(token)))                                  \
		return result;

Writer<CST::CST*> Parser::parse_top_level() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse top level program"}};

	auto declarations = parse_declaration_list(TokenTag::END);
	CHECK_AND_RETURN(result, declarations);

	auto e = m_cst_allocator->make<CST::DeclarationList>();
	e->m_declarations = std::move(declarations.m_result);

	return make_writer<CST::CST*>(e);
}

Writer<std::vector<CST::Declaration>>
Parser::parse_declaration_list(TokenTag terminator) {
	Writer<std::vector<CST::Declaration>> result = {
	    {"Parse Error: Failed to parse declaration list"}};

	std::vector<CST::Declaration> declarations;

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
		CHECK_AND_RETURN(result, declaration);

		declarations.push_back(std::move(*declaration.m_result));
	}

	return make_writer(std::move(declarations));
}

Writer<std::vector<CST::CST*>> Parser::parse_expression_list(
    TokenTag delimiter, TokenTag terminator, bool allow_trailing_delimiter) {
	Writer<std::vector<CST::CST*>> result = {
	    {"Parse Error: Failed to parse expression list"}};

	std::vector<CST::CST*> expressions;

	if (peek()->m_type == terminator) {
		advance_token_cursor();
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
					advance_token_cursor();
					break;
				} else {
					result.m_error.m_sub_errors.push_back(
					    {{"Found a terminator after a delimiter in an "
					      "expression list "}});
					return result;
				}
			}

			auto expression = parse_expression();
			CHECK_AND_RETURN(result, expression);

			expressions.push_back(expression.m_result);

			auto p1 = peek();

			if (p1->m_type == delimiter) {
				advance_token_cursor();
			} else if (p1->m_type == terminator) {
				advance_token_cursor();
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

Writer<CST::Declaration*> Parser::parse_declaration() {
	Writer<CST::Declaration*> result = {
	    {"Parse Error: Failed to parse declaration"}};

	Writer<Token const*> name = require(TokenTag::IDENTIFIER);
	CHECK_AND_RETURN(result, name);

	Writer<CST::CST*> type;

	if (consume(TokenTag::DECLARE_ASSIGN)) {
	} else if (consume(TokenTag::DECLARE)) {
		type = parse_type_term();
		CHECK_AND_RETURN(result, type);
		REQUIRE(result, TokenTag::ASSIGN);
	} else {
		result.m_error.m_sub_errors.push_back(
		    make_expected_error("':' or ':='", peek()));
	}

	auto value = parse_expression();
	CHECK_AND_RETURN(result, value);
	REQUIRE(result, TokenTag::SEMICOLON);

	auto p = m_cst_allocator->make<CST::Declaration>();
	p->m_data = {
		name.m_result,
		type.m_result,
		value.m_result,
	};

	return make_writer<CST::Declaration*>(p);
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
		exit(1);
		assert(false);
	}
}

Writer<std::vector<CST::CST*>> Parser::parse_argument_list() {
	Writer<std::vector<CST::CST*>> result = {
	    {"Parse Error: Failed to argument list"}};

	REQUIRE(result, TokenTag::PAREN_OPEN);
	auto args =
	    parse_expression_list(TokenTag::COMMA, TokenTag::PAREN_CLOSE, false);
	CHECK_AND_RETURN(result, args);

	return args;
}

/* The algorithm used here is called 'Pratt Parsing'
 * Here is an article with more information:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
Writer<CST::CST*> Parser::parse_expression(int bp, CST::CST* parsed_lhs) {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse expression"}};

	Writer<CST::CST*> lhs;
	if (!parsed_lhs) {
		lhs = parse_terminal();
		CHECK_AND_RETURN(result, lhs);
	} else {
		lhs = make_writer(parsed_lhs);
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
			CHECK_AND_RETURN(result, args);

			auto e = m_cst_allocator->make<CST::CallExpression>();
			e->m_callee = lhs.m_result;
			e->m_args = std::move(args.m_result);
			lhs.m_result = e;

			continue;
		}

		if (op->m_type == TokenTag::BRACKET_OPEN) {
			advance_token_cursor();

			auto index = parse_expression();
			CHECK_AND_RETURN(result, index);
			REQUIRE(result, TokenTag::BRACKET_CLOSE);

			auto e = m_cst_allocator->make<CST::IndexExpression>();
			e->m_callee = lhs.m_result;
			e->m_index = index.m_result;
			lhs.m_result = e;

			continue;
		}

		if (match(TokenTag::POLY_OPEN)) {
			auto args = parse_type_term_arguments();
			CHECK_AND_RETURN(result, args);

			auto e = m_cst_allocator->make<CST::TypeTerm>();
			e->m_callee = lhs.m_result;
			e->m_args = std::move(args.m_result);
			lhs.m_result = e;

			continue;
		}

		if (consume(TokenTag::DOT)) {
			auto member = require(TokenTag::IDENTIFIER);
			CHECK_AND_RETURN(result, member);

			auto e = m_cst_allocator->make<CST::AccessExpression>();
			e->m_record = lhs.m_result;
			e->m_member = member.m_result;
			lhs.m_result = e;

			continue;
		}

		if (consume(TokenTag::BRACE_OPEN)) {
			auto args =
			    parse_expression_list(TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true);
			CHECK_AND_RETURN(result, args);

			auto e = m_cst_allocator->make<CST::ConstructorExpression>();
			e->m_constructor = lhs.m_result;
			e->m_args = std::move(args.m_result);
			lhs.m_result = e;

			continue;
		}

		advance_token_cursor();
		auto rhs = parse_expression(rp);

		auto e = m_cst_allocator->make<CST::BinaryExpression>();
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
Writer<CST::CST*> Parser::parse_terminal() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse terminal expression"}};

	auto token = peek();

	if (token->m_type == TokenTag::KEYWORD_NULL) {
		auto e = m_cst_allocator->make<CST::NullLiteral>();
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::KEYWORD_TRUE) {
		auto e = m_cst_allocator->make<CST::BooleanLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::KEYWORD_FALSE) {
		auto e = m_cst_allocator->make<CST::BooleanLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::SUB || token->m_type == TokenTag::ADD) {
		advance_token_cursor();

		// NOTE: we store the sign token of the source code for future
		// feature of printing the source code when an error occurs
		if (match(TokenTag::INTEGER)) {
			auto e = m_cst_allocator->make<CST::IntegerLiteral>();
			e->m_negative = token->m_type == TokenTag::SUB;
			e->m_sign = token;
			e->m_token = peek();
			advance_token_cursor();
			return make_writer<CST::CST*>(e);
		} else if (match(TokenTag::NUMBER)) {
			auto e = m_cst_allocator->make<CST::NumberLiteral>();
			e->m_negative = token->m_type == TokenTag::SUB;
			e->m_sign = token;
			e->m_token = peek();
			advance_token_cursor();
			return make_writer<CST::CST*>(e);
		}

		return token->m_type == TokenTag::SUB
		       ? Writer<CST::CST*> {{"Parse Error: Stray minus sign with no number"}}
		       : Writer<CST::CST*> {{"Parse Error: Stray plus sign with no number"}};
	}

	if (token->m_type == TokenTag::INTEGER) {
		auto e = m_cst_allocator->make<CST::IntegerLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::NUMBER) {
		auto e = m_cst_allocator->make<CST::NumberLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::IDENTIFIER) {
		auto e = m_cst_allocator->make<CST::Identifier>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::STRING) {
		auto e = m_cst_allocator->make<CST::StringLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer<CST::CST*>(e);
	}

	if (token->m_type == TokenTag::KEYWORD_FN) {
		auto function = parse_function();
		CHECK_AND_RETURN(result, function);
		return function;
	}

	if (token->m_type == TokenTag::KEYWORD_IF) {
		auto ternary = parse_ternary_expression();
		CHECK_AND_RETURN(result, ternary);
		return ternary;
	}

	// parse a parenthesized expression.
	if (token->m_type == TokenTag::PAREN_OPEN) {
		advance_token_cursor();
		auto expr = parse_expression();
		CHECK_AND_RETURN(result, expr);
		REQUIRE(result, TokenTag::PAREN_CLOSE);
		return expr;
	}

	if (token->m_type == TokenTag::KEYWORD_ARRAY) {
		auto array = parse_array_literal();
		CHECK_AND_RETURN(result, array);
		return array;
	}

	if (token->m_type == TokenTag::KEYWORD_UNION ||
	    token->m_type == TokenTag::KEYWORD_STRUCT) {
		// TODO: do the other type functions
		auto type = parse_type_function();
		CHECK_AND_RETURN(result, type);
		return type;
	}

	if (token->m_type == TokenTag::KEYWORD_MATCH) {
		auto match_expr = parse_match_expression();
		CHECK_AND_RETURN(result, match_expr);
		return match_expr;
	}

	if (token->m_type == TokenTag::KEYWORD_SEQ) {
		auto expr = parse_sequence_expression();
		CHECK_AND_RETURN(result, expr);
		return expr;
	}

	result.m_error.m_sub_errors.push_back(
	    {make_expected_error(token_string[int(TokenTag::KEYWORD_FN)], token)});

	result.m_error.m_sub_errors.push_back(
	    {make_expected_error(token_string[int(TokenTag::IDENTIFIER)], token)});

	result.m_error.m_sub_errors.push_back(
	    {make_expected_error(token_string[int(TokenTag::NUMBER)], token)});

	return result;
}

Writer<CST::CST*> Parser::parse_ternary_expression(CST::CST* parsed_condition) {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse ternary expression"}};

	Writer<CST::CST*> condition;
	if (!parsed_condition) {
		REQUIRE(result, TokenTag::KEYWORD_IF);
		REQUIRE(result, TokenTag::PAREN_OPEN);

		condition = parse_expression();
		CHECK_AND_RETURN(result, condition);
		REQUIRE(result, TokenTag::PAREN_CLOSE);
	} else {
		condition = make_writer(parsed_condition);
	}

	REQUIRE(result, TokenTag::KEYWORD_THEN);

	auto then_expr = parse_expression();
	CHECK_AND_RETURN(result, then_expr);
	REQUIRE(result, TokenTag::KEYWORD_ELSE);

	auto else_expr = parse_expression();
	CHECK_AND_RETURN(result, else_expr);

	auto e = m_cst_allocator->make<CST::TernaryExpression>();
	e->m_condition = condition.m_result;
	e->m_then_expr = then_expr.m_result;
	e->m_else_expr = else_expr.m_result;

	return make_writer<CST::CST*>(e);
}

Writer<CST::Identifier*> Parser::parse_identifier(bool types_allowed) {
	Writer<CST::Identifier*> result = {
	    {"Parse Error: Failed to parse identifier"}};

	Token const* token;

	if (types_allowed and match(TokenTag::KEYWORD_ARRAY)) {
		token = peek();
		advance_token_cursor();
	} else {
		auto identifier = require(TokenTag::IDENTIFIER);
		CHECK_AND_RETURN(result, identifier);
		token = identifier.m_result;
	}

	auto e = m_cst_allocator->make<CST::Identifier>();
	e->m_token = token;

	return make_writer<CST::Identifier*>(e);
}

Writer<CST::CST*> Parser::parse_array_literal() {
	Writer<CST::CST*> result = {
	    {"Failed to parse array literal"}};

	REQUIRE(result, TokenTag::KEYWORD_ARRAY);
	REQUIRE(result, TokenTag::BRACE_OPEN);

	auto elements = parse_expression_list(
	    TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true);
	CHECK_AND_RETURN(result, elements);

	auto e = m_cst_allocator->make<CST::ArrayLiteral>();
	e->m_elements = std::move(elements.m_result);

	return make_writer<CST::CST*>(e);
}

/*
 * functions look like this:
 * fn (x : int, y, z : string) {
 *   print(x);
 * }
 */
Writer<CST::CST*> Parser::parse_function() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse function"}};

	REQUIRE(result, TokenTag::KEYWORD_FN);
	REQUIRE(result, TokenTag::PAREN_OPEN);

	std::vector<CST::Declaration> args;
	while (1) {
		if (consume(TokenTag::PAREN_CLOSE)) {
			break;
		} else if (match(TokenTag::IDENTIFIER)) {
			// consume argument name

			CST::Declaration arg;

			arg.m_data.m_identifier_token = peek();
			advance_token_cursor();

			if (consume(TokenTag::DECLARE)) {
				// optionally consume a type hint
				auto type = parse_type_term();
				CHECK_AND_RETURN(result, type);
				arg.m_data.m_type_hint = type.m_result;
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
		CHECK_AND_RETURN(result, expression);

		auto e = m_cst_allocator->make<CST::FunctionLiteral>();
		e->m_body = expression.m_result;
		e->m_args = std::move(args);

		return make_writer<CST::CST*>(e);
	} else {
		auto block = parse_block();
		CHECK_AND_RETURN(result, block);

		auto e = m_cst_allocator->make<CST::BlockFunctionLiteral>();
		e->m_body = block.m_result;

		std::vector<CST::DeclarationData> args_data;
		for(auto& arg : args)
			args_data.push_back(arg.m_data);

		e->m_args = std::move(args_data);

		return make_writer<CST::CST*>(e);
	}
}

Writer<CST::CST*> Parser::parse_block() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse block statement"}};

	REQUIRE(result, TokenTag::BRACE_OPEN);

	std::vector<CST::CST*> statements;

	// loop until we find a matching closing bracket
	while (1) {
		auto p0 = peek();

		if (p0->m_type == TokenTag::END) {
			result.m_error.m_sub_errors.push_back(
			    {{"Found EOF while parsing block statement"}});
			return result;
		}

		if (p0->m_type == TokenTag::BRACE_CLOSE) {
			advance_token_cursor();
			break;
		}

		auto statement = parse_statement();
		CHECK_AND_RETURN(result, statement);
		statements.push_back(statement.m_result);
	}

	auto e = m_cst_allocator->make<CST::Block>();
	e->m_body = std::move(statements);

	return make_writer<CST::CST*>(e);
}

Writer<CST::CST*> Parser::parse_return_statement() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse return statement"}};

	REQUIRE(result, TokenTag::KEYWORD_RETURN);

	auto value = parse_expression();
	CHECK_AND_RETURN(result, value);
	REQUIRE(result, TokenTag::SEMICOLON);

	auto e = m_cst_allocator->make<CST::ReturnStatement>();
	e->m_value = value.m_result;

	return make_writer<CST::CST*>(e);
}
Writer<CST::CST*> Parser::parse_if_else_stmt_or_expr() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse if-else statement or expression"}};

	REQUIRE(result, TokenTag::KEYWORD_IF);
	REQUIRE(result, TokenTag::PAREN_OPEN);

	auto condition = parse_expression();
	CHECK_AND_RETURN(result, condition);
	REQUIRE(result, TokenTag::PAREN_CLOSE);

	if (match(TokenTag::KEYWORD_THEN)) {
		// rollback to whole expression
		auto ternary = parse_ternary_expression(condition.m_result);
		CHECK_AND_RETURN(result, ternary);

		auto expression = parse_expression(0, ternary.m_result);
		CHECK_AND_RETURN(result, expression);
		REQUIRE(result, TokenTag::SEMICOLON);

		return expression;
	}

	auto body = parse_statement();
	CHECK_AND_RETURN(result, body);

	auto e = m_cst_allocator->make<CST::IfElseStatement>();
	e->m_condition = condition.m_result;
	e->m_body = body.m_result;

	if (consume(TokenTag::KEYWORD_ELSE)) {
		auto else_body = parse_statement();
		CHECK_AND_RETURN(result, else_body);

		e->m_else_body = else_body.m_result;
	}

	return make_writer<CST::CST*>(e);
}

Writer<CST::CST*> Parser::parse_for_statement() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse for statement"}};

	REQUIRE(result, TokenTag::KEYWORD_FOR);
	REQUIRE(result, TokenTag::PAREN_OPEN);

	// NOTE: handles semicolon already
	auto declaration = parse_declaration();
	CHECK_AND_RETURN(result, declaration);

	auto condition = parse_expression();
	CHECK_AND_RETURN(result, condition);
	REQUIRE(result, TokenTag::SEMICOLON);

	auto action = parse_expression();
	CHECK_AND_RETURN(result, action);
	REQUIRE(result, TokenTag::PAREN_CLOSE);

	auto body = parse_statement();
	CHECK_AND_RETURN(result, body);

	auto e = m_cst_allocator->make<CST::ForStatement>();
	e->m_declaration = std::move(*declaration.m_result);
	e->m_condition = condition.m_result;
	e->m_action = action.m_result;
	e->m_body = body.m_result;

	return make_writer<CST::CST*>(e);
}

Writer<CST::CST*> Parser::parse_while_statement() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse while statement"}};

	REQUIRE(result, TokenTag::KEYWORD_WHILE);
	REQUIRE(result, TokenTag::PAREN_OPEN);

	auto condition = parse_expression();
	CHECK_AND_RETURN(result, condition);
	REQUIRE(result, TokenTag::PAREN_CLOSE);

	auto body = parse_statement();
	CHECK_AND_RETURN(result, body);

	auto e = m_cst_allocator->make<CST::WhileStatement>();
	e->m_condition = condition.m_result;
	e->m_body = body.m_result;

	return make_writer<CST::CST*>(e);
}

Writer<CST::CST*> Parser::parse_match_expression() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse match expression"}};

	REQUIRE(result, TokenTag::KEYWORD_MATCH);
	REQUIRE(result, TokenTag::PAREN_OPEN);

	auto matchee_and_hint = parse_name_and_type();
	CHECK_AND_RETURN(result, matchee_and_hint);
	REQUIRE(result, TokenTag::PAREN_CLOSE);
	REQUIRE(result, TokenTag::BRACE_OPEN);

	std::vector<CST::MatchExpression::CaseData> cases;
	while (!consume(TokenTag::BRACE_CLOSE)) {
		auto case_name = require(TokenTag::IDENTIFIER);
		CHECK_AND_RETURN(result, case_name);
		REQUIRE(result, TokenTag::BRACE_OPEN);

		auto name_and_type = parse_name_and_type();
		CHECK_AND_RETURN(result, name_and_type);
		REQUIRE(result, TokenTag::BRACE_CLOSE);
		REQUIRE(result, TokenTag::ARROW);

		auto expression = parse_expression();
		CHECK_AND_RETURN(result, expression);
		REQUIRE(result, TokenTag::SEMICOLON);

		cases.push_back(
		    {case_name.m_result,
		     name_and_type.m_result.first,
		     name_and_type.m_result.second,
		     expression.m_result});
	}

	CST::Identifier matchee;
	matchee.m_token = matchee_and_hint.m_result.first;

	auto match = m_cst_allocator->make<CST::MatchExpression>();
	match->m_matchee = std::move(matchee);
	match->m_type_hint = matchee_and_hint.m_result.second;
	match->m_cases = std::move(cases);

	return make_writer<CST::CST*>(match);
}

Writer<CST::CST*> Parser::parse_sequence_expression() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse sequence expression"}};

	REQUIRE(result, TokenTag::KEYWORD_SEQ);
	auto body = parse_block();
	CHECK_AND_RETURN(result, body);

	auto expr = m_cst_allocator->make<CST::SequenceExpression>();
	expr->m_body = static_cast<CST::Block*>(body.m_result);

	return make_writer<CST::CST*>(expr);
}

Writer<std::pair<Token const*, CST::CST*>> Parser::parse_name_and_type(bool required_type) {
	Writer<std::pair<Token const*, CST::CST*>> result = {
	    {"Parse Error: Failed to parse name and type"}};

	auto name = require(TokenTag::IDENTIFIER);
	CHECK_AND_RETURN(result, name);

	Writer<CST::CST*> type;
	if (required_type or match(TokenTag::DECLARE)) {
		REQUIRE(result, TokenTag::DECLARE);

		type = parse_type_term();
		CHECK_AND_RETURN(result, type);
	}

	return make_writer<std::pair<Token const*, CST::CST*>>(
	    {name.m_result, type.m_result});
}

/*
 * Looks ahead a few tokens to predict what syntactic structure we are about to
 * parse. This prevents us from backtracking and ensures the parser runs in
 * linear time.
 */
Writer<CST::CST*> Parser::parse_statement() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse statement"}};

	auto* p0 = peek(0);
	if (p0->m_type == TokenTag::IDENTIFIER) {
		auto* p1 = peek(1);

		if (p1->m_type == TokenTag::DECLARE ||
		    p1->m_type == TokenTag::DECLARE_ASSIGN) {

			auto declaration = parse_declaration();
			CHECK_AND_RETURN(result, declaration);

			return make_writer<CST::CST*>(declaration.m_result);
		} else {
			auto expression = parse_expression();
			CHECK_AND_RETURN(result, expression);
			REQUIRE(result, TokenTag::SEMICOLON);

			return expression;
		}
	} else if (p0->m_type == TokenTag::KEYWORD_RETURN) {
		auto return_statement = parse_return_statement();
		CHECK_AND_RETURN(result, return_statement);
		return return_statement;
	} else if (p0->m_type == TokenTag::KEYWORD_IF) {
		auto if_else_stmt_or_expr = parse_if_else_stmt_or_expr();
		CHECK_AND_RETURN(result, if_else_stmt_or_expr);
		return if_else_stmt_or_expr;
	} else if (p0->m_type == TokenTag::KEYWORD_FOR) {
		auto for_statement = parse_for_statement();
		CHECK_AND_RETURN(result, for_statement);
		return for_statement;
	} else if (p0->m_type == TokenTag::KEYWORD_WHILE) {
		auto while_statement = parse_while_statement();
		CHECK_AND_RETURN(result, while_statement);
		return while_statement;
	} else if (p0->m_type == TokenTag::BRACE_OPEN) {
		auto block_statement = parse_block();
		CHECK_AND_RETURN(result, block_statement);
		return block_statement;
	} else {
		auto expression = parse_expression();
		CHECK_AND_RETURN(result, expression);
		REQUIRE(result, TokenTag::SEMICOLON);
		return expression;
	}

	return result;
}

Writer<std::vector<CST::CST*>> Parser::parse_type_term_arguments() {
	Writer<std::vector<CST::CST*>> result = {
	    {"Parse Error: Failed to parse type arguments"}};

	REQUIRE(result, TokenTag::POLY_OPEN);

	std::vector<CST::CST*> args;

	while (!consume(TokenTag::POLY_CLOSE)) {
		auto arg = parse_type_term();
		CHECK_AND_RETURN(result, arg);

		args.push_back(arg.m_result);
	}

	return make_writer(std::move(args));
}

Writer<CST::CST*> Parser::parse_type_term() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse type"}};

	auto callee = parse_identifier(true);
	CHECK_AND_RETURN(result, callee);

	if (!match(TokenTag::POLY_OPEN))
		return make_writer<CST::CST*>(callee.m_result);

	auto args = parse_type_term_arguments();
	CHECK_AND_RETURN(result, args);

	auto e = m_cst_allocator->make<CST::TypeTerm>();
	e->m_callee = callee.m_result;
	e->m_args = std::move(args.m_result);
	return make_writer<CST::CST*>(e);
}

Writer<std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>> Parser::parse_type_list(
    bool with_identifiers = false) {
	Writer<std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>>
	    result = {{"Parse Error: Failed to parse type list"}};

	std::vector<CST::Identifier> identifiers;
	std::vector<CST::CST*> types;

	REQUIRE(result, TokenTag::BRACE_OPEN);

	while(!consume(TokenTag::BRACE_CLOSE)) {
		if (with_identifiers) {
			auto cons = parse_identifier();
			CHECK_AND_RETURN(result, cons);
			REQUIRE(result, TokenTag::DECLARE);

			identifiers.push_back(std::move(*cons.m_result));
		}

		auto type = parse_type_term();
		CHECK_AND_RETURN(result, type);
		REQUIRE(result, TokenTag::SEMICOLON);

		types.push_back(type.m_result);
	}

	return make_writer<
	    std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>>(
	    make_pair(std::move(identifiers), std::move(types)));
}

Writer<CST::CST*> Parser::parse_type_var() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse type var"}};

	REQUIRE(result, TokenTag::AT);

	auto token = require(TokenTag::IDENTIFIER);
	CHECK_AND_RETURN(result, token);

	auto t = m_cst_allocator->make<CST::TypeVar>();
	t->m_token = token.m_result;

	return make_writer<CST::CST*>(t);
}

Writer<CST::CST*> Parser::parse_type_function() {
	Writer<CST::CST*> result = {
	    {"Parse Error: Failed to parse type function"}};

	if (consume(TokenTag::KEYWORD_UNION)) {
		auto tl = parse_type_list(true);
		CHECK_AND_RETURN(result, tl);

		auto u = m_cst_allocator->make<CST::UnionExpression>();
		u->m_constructors = std::move(tl.m_result.first);
		u->m_types = std::move(tl.m_result.second);

		return make_writer<CST::CST*>(u);
	} else if (consume(TokenTag::KEYWORD_TUPLE)) {
		auto tl = parse_type_list(false);
		CHECK_AND_RETURN(result, tl);

		auto t = m_cst_allocator->make<CST::TupleExpression>();
		t->m_types = std::move(tl.m_result.second);

		return make_writer<CST::CST*>(t);
	} else if (consume(TokenTag::KEYWORD_STRUCT)) {
		auto tl = parse_type_list(true);
		CHECK_AND_RETURN(result, tl);

		auto s = m_cst_allocator->make<CST::StructExpression>();
		s->m_fields = std::move(tl.m_result.first);
		s->m_types = std::move(tl.m_result.second);

		return make_writer<CST::CST*>(s);
	}

	return result;
}

#undef CHECK_AND_RETURN
#undef REQUIRE

Writer<CST::CST*> parse_program(TokenArray const& ta, CST::Allocator& allocator) {
	Parser p {ta, &allocator};
	return p.parse_top_level();
}

Writer<CST::CST*> parse_expression(TokenArray const& ta, CST::Allocator& allocator) {
	Parser p {ta, &allocator};
	return p.parse_expression();
}
