#include "parser.hpp"

#include "./utils/error_report.hpp"
#include "./utils/string_view.hpp"
#include "./utils/writer.hpp"
#include "cst.hpp"
#include "cst_allocator.hpp"
#include "frontend_context.hpp"
#include "lexer_result.hpp"
#include "token_array.hpp"

#include <sstream>
#include <utility>
#include <vector>

#include <cassert>

template <typename T>
Writer<T> make_writer(T x) {
	return {{}, std::move(x)};
}


struct Parser {
	/* token handler */
	TokenArray const& m_tokens;
	Frontend::Context const& m_file_context;
	CST::Allocator& m_cst_allocator;
	int m_token_cursor { 0 };

	Parser(TokenArray const& tokens, Frontend::Context const& file_context, CST::Allocator& cst_allocator)
	    : m_tokens {tokens}
		, m_file_context {file_context}
	    , m_cst_allocator {cst_allocator} {}

	Writer<std::vector<CST::CST*>> parse_expression_list(TokenTag, TokenTag, bool);

	Writer<CST::CST*> parse_top_level();

	Writer<CST::CST*> parse_sequence_expression();
	Writer<CST::Identifier*> parse_term_identifier();
	Writer<CST::Identifier*> parse_type_identifier();

	Writer<CST::Declaration*> parse_declaration();
	Writer<CST::Declaration*> parse_func_declaration();
	Writer<CST::PlainDeclaration*> parse_plain_declaration();
	Writer<CST::DeclarationData> parse_plain_declaration_data();

	Writer<CST::CST*> parse_full_expression(int bp = 0);
	Writer<CST::CST*> parse_expression(int bp = 0);
	Writer<CST::CST*> parse_expression(CST::CST* lhs, int bp = 0);

	Writer<CST::CST*> parse_terminal();
	Writer<CST::CST*> parse_if_else_expression();
	Writer<CST::CST*> parse_if_else_expression(CST::CST* parsed_condition);
	Writer<CST::FuncParameters> parse_function_parameters();
	Writer<CST::CST*> parse_function();
	Writer<CST::CST*> parse_array_literal();
	Writer<std::vector<CST::CST*>> parse_argument_list();
	Writer<CST::Block*> parse_block();
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

	ErrorReport make_located_error(string_view text, Token const* token) {
		SourceLocation token_location = m_file_context.char_offset_to_location(token->m_start_offset);
		return ::make_located_error(text, token_location);
	}

	ErrorReport make_expected_error(string_view expected, Token const* found_token) {
		std::stringstream ss;

		ss << "Expected " << expected << " but got ";

		if (found_token->m_type == TokenTag::END) {
			ss << "to the end of the file";
		} else {
			ss << token_string[int(found_token->m_type)] << ' ' << found_token->m_text;
		}

		ss << " instead";

		return make_located_error(ss.str(), found_token);
	}

	ErrorReport make_expected_error(TokenTag tag, Token const* found_token) {
		return make_expected_error(token_string[int(tag)], found_token);
	}

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
			return {make_expected_error(expected_type, current_token)};
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

// These macros use the non-standard "statement expression" syntax, a GNU
// extension supported by GCC, Clang, and Intel compilers.
#define TRY(x)                                                                 \
	({                                                                         \
		auto _temporary = x;                                                   \
		if (not _temporary.ok())                                               \
			return std::move(_temporary).error();                              \
		_temporary.m_result;                                                   \
	})

#define TRY_WITH(lhs, x)                                                       \
	({                                                                         \
		auto _temporary = x;                                                   \
		if (not _temporary.ok()) {                                             \
			lhs.add_sub_error(std::move(_temporary).error());                  \
			return lhs;                                                        \
		}                                                                      \
		_temporary.m_result;                                                   \
	})

#define REQUIRE(token) TRY(require(token))

#define REQUIRE_WITH(result, token) TRY_WITH(result, require(token))

Writer<CST::CST*> Parser::parse_top_level() {

	std::vector<CST::Declaration*> declarations;
	while (!match(TokenTag::END)) {
		declarations.push_back(TRY(parse_declaration()));
	}

	auto e = m_cst_allocator.make<CST::Program>();
	e->m_declarations = std::move(declarations);
	return make_writer(e);
}

Writer<std::vector<CST::CST*>> Parser::parse_expression_list(
    TokenTag delimiter, TokenTag terminator, bool allow_trailing_delimiter) {

	std::vector<CST::CST*> expressions;

	if (consume(terminator))
		return make_writer(std::move(expressions));

	while (1) {

		expressions.push_back(TRY(parse_expression()));

		auto p0 = peek();

		if (p0->m_type == delimiter) {
			advance_token_cursor();

			if (match(terminator)) {
				if (allow_trailing_delimiter) {
					advance_token_cursor();
					break;
				}

				return make_located_error(
				    "Found trailing delimiter in expression list", p0);
			}
		} else if (p0->m_type == terminator) {
			advance_token_cursor();
			break;
		} else {
			ErrorReport result = {{"Unexpected token"}};
			result.add_sub_error(make_expected_error(delimiter, p0));
			result.add_sub_error(make_expected_error(terminator, p0));
			return result;
		}
	}

	return make_writer(std::move(expressions));
}

Writer<CST::Declaration*> Parser::parse_declaration() {
	if (match(TokenTag::KEYWORD_FN))
		return parse_func_declaration();

	if (match(TokenTag::IDENTIFIER))
		return parse_plain_declaration();

	return make_expected_error("an identifier or 'fn'", peek());
}

Writer<CST::Declaration*> Parser::parse_func_declaration() {
	ErrorReport result = {{"Failed to parse function declaration"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_FN);

	auto identifier = REQUIRE_WITH(result, TokenTag::IDENTIFIER);

	auto args = TRY_WITH(result, parse_function_parameters());

	if (consume(TokenTag::ARROW)) {
		auto expression = TRY_WITH(result, parse_expression());

		REQUIRE_WITH(result, TokenTag::SEMICOLON);

		auto p = m_cst_allocator.make<CST::FuncDeclaration>();

		p->m_identifier = identifier;
		p->m_args = std::move(args);
		p->m_body = expression;

		return make_writer(p);
	}

	if (match(TokenTag::BRACE_OPEN)) {
		auto block = TRY_WITH(result, parse_block());

		REQUIRE_WITH(result, TokenTag::SEMICOLON);

		auto p = m_cst_allocator.make<CST::BlockFuncDeclaration>();

		p->m_identifier = identifier;
		p->m_args = std::move(args);
		p->m_body = block;

		return make_writer(p);
	}

	result.add_sub_error(make_expected_error("'=>' or '{'", peek()));
	return result;
}

Writer<CST::PlainDeclaration*> Parser::parse_plain_declaration() {
	ErrorReport result = {{"Failed to parse declaration"}};

	auto decl_data = TRY_WITH(result, parse_plain_declaration_data());

	auto p = m_cst_allocator.make<CST::PlainDeclaration>();
	p->m_data = decl_data;

	return make_writer(p);
}



Writer<CST::DeclarationData> Parser::parse_plain_declaration_data() {
	auto name = REQUIRE(TokenTag::IDENTIFIER);

	CST::CST* type = nullptr;

	if (consume(TokenTag::DECLARE_ASSIGN)) {
	} else if (consume(TokenTag::DECLARE)) {
		type = TRY(parse_type_term());
		REQUIRE(TokenTag::ASSIGN);
	} else {
		return {make_expected_error("':' or ':='", peek())};
	}

	auto value = TRY(parse_full_expression());

	REQUIRE(TokenTag::SEMICOLON);

	return make_writer<CST::DeclarationData>({name, type, value});
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
	case TokenTag::LOGIC_AND:
	case TokenTag::LOGIC_IOR:
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
	case TokenTag::LOGIC_IOR:
	case TokenTag::LOGIC_AND:
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
	ErrorReport result = {{"Failed to parse argument list"}};

	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);
	auto args = TRY_WITH(
	    result,
	    parse_expression_list(TokenTag::COMMA, TokenTag::PAREN_CLOSE, false));

	return make_writer(args);
}

// This function just wraps the result of parse_expression in an extra layer of error when it fails
Writer<CST::CST*> Parser::parse_full_expression(int bp) {
	ErrorReport result = {{"Failed to parse expression"}};
	auto expr = TRY_WITH(result, parse_expression(bp));
	return make_writer(expr);
}

Writer<CST::CST*> Parser::parse_expression(int bp) {
	return parse_expression(TRY(parse_terminal()), bp);
}

/* The algorithm used here is called 'Pratt Parsing'
 * Here is an article with more information:
 * https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
 */
Writer<CST::CST*> Parser::parse_expression(CST::CST* lhs, int bp) {
	assert(lhs);
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

		if (not is_binary_operator(op->m_type))
			return make_expected_error("a binary operator", op);

		auto op_bp = binding_power_of(op->m_type);
		auto& lp = op_bp.left;
		auto& rp = op_bp.right;

		if (lp < bp)
			break;

		if (op->m_type == TokenTag::PAREN_OPEN) {
			auto args = TRY(parse_argument_list());

			auto e = m_cst_allocator.make<CST::CallExpression>();
			e->m_callee = lhs;
			e->m_args = std::move(args);
			lhs = e;

			continue;
		}

		if (op->m_type == TokenTag::BRACKET_OPEN) {
			advance_token_cursor();

			auto index = TRY(parse_expression());

			REQUIRE(TokenTag::BRACKET_CLOSE);

			auto e = m_cst_allocator.make<CST::IndexExpression>();
			e->m_callee = lhs;
			e->m_index = index;
			lhs = e;

			continue;
		}

		if (match(TokenTag::POLY_OPEN)) {
			auto args = TRY(parse_type_term_arguments());

			auto e = m_cst_allocator.make<CST::TypeTerm>();
			e->m_callee = lhs;
			e->m_args = std::move(args);
			lhs = e;

			continue;
		}

		if (consume(TokenTag::DOT)) {
			auto member = REQUIRE(TokenTag::IDENTIFIER);

			auto e = m_cst_allocator.make<CST::AccessExpression>();
			e->m_record = lhs;
			e->m_member = member;
			lhs = e;

			continue;
		}

		if (consume(TokenTag::BRACE_OPEN)) {
			auto args = TRY(parse_expression_list(
			    TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true));

			auto e = m_cst_allocator.make<CST::ConstructorExpression>();
			e->m_constructor = lhs;
			e->m_args = std::move(args);
			lhs = e;

			continue;
		}

		advance_token_cursor();
		auto rhs = TRY(parse_expression(rp));

		auto e = m_cst_allocator.make<CST::BinaryExpression>();
		e->m_op_token = op;
		e->m_lhs = lhs;
		e->m_rhs = rhs;

		lhs = e;
	}

	return make_writer(lhs);
}

/* We say a terminal is any expression that is not an infix expression.
 * This is not what the term usually means in the literature.
 */
Writer<CST::CST*> Parser::parse_terminal() {
	auto token = peek();

	if (token->m_type == TokenTag::KEYWORD_NULL) {
		auto e = m_cst_allocator.make<CST::NullLiteral>();
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::KEYWORD_TRUE) {
		auto e = m_cst_allocator.make<CST::BooleanLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::KEYWORD_FALSE) {
		auto e = m_cst_allocator.make<CST::BooleanLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::SUB || token->m_type == TokenTag::ADD) {
		advance_token_cursor();

		// NOTE: we store the sign token of the source code for future
		// feature of printing the source code when an error occurs
		if (match(TokenTag::INTEGER)) {
			auto e = m_cst_allocator.make<CST::IntegerLiteral>();
			e->m_negative = token->m_type == TokenTag::SUB;
			e->m_sign = token;
			e->m_token = peek();
			advance_token_cursor();
			return make_writer(e);
		} else if (match(TokenTag::NUMBER)) {
			auto e = m_cst_allocator.make<CST::NumberLiteral>();
			e->m_negative = token->m_type == TokenTag::SUB;
			e->m_sign = token;
			e->m_token = peek();
			advance_token_cursor();
			return make_writer(e);
		}

		return token->m_type == TokenTag::SUB
		           ? make_located_error("Stray minus sign with no number", token)
		           : make_located_error("Stray plus sign with no number", token);
	}

	if (token->m_type == TokenTag::INTEGER) {
		auto e = m_cst_allocator.make<CST::IntegerLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::NUMBER) {
		auto e = m_cst_allocator.make<CST::NumberLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::IDENTIFIER) {
		auto e = m_cst_allocator.make<CST::Identifier>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::STRING) {
		auto e = m_cst_allocator.make<CST::StringLiteral>();
		e->m_token = token;
		advance_token_cursor();
		return make_writer(e);
	}

	if (token->m_type == TokenTag::KEYWORD_FN) {
		return parse_function();
	}

	if (token->m_type == TokenTag::KEYWORD_IF) {
		return parse_if_else_expression();
	}

	// parse a parenthesized expression.
	if (token->m_type == TokenTag::PAREN_OPEN) {
		advance_token_cursor();
		auto expr = TRY(parse_expression());
		REQUIRE(TokenTag::PAREN_CLOSE);
		return make_writer(expr);
	}

	if (token->m_type == TokenTag::KEYWORD_ARRAY) {
		return parse_array_literal();
	}

	if (token->m_type == TokenTag::KEYWORD_UNION ||
	    token->m_type == TokenTag::KEYWORD_STRUCT) {
		// TODO: do the other type functions
		return parse_type_function();
	}

	if (token->m_type == TokenTag::KEYWORD_MATCH) {
		return parse_match_expression();
	}

	if (token->m_type == TokenTag::KEYWORD_SEQ) {
		return parse_sequence_expression();
	}

	return make_expected_error("a literal, a conditional expression, or an identifier", token);
}

Writer<CST::CST*> Parser::parse_if_else_expression() {
	ErrorReport result = {{"Failed to parse if-else expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_IF);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	CST::CST* condition = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);

	return parse_if_else_expression(condition);
}

Writer<CST::CST*> Parser::parse_if_else_expression(CST::CST* condition) {
	assert(condition);

	ErrorReport result = {{"Failed to parse if-else expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_THEN);
	auto then_expr = TRY(parse_expression());

	REQUIRE_WITH(result, TokenTag::KEYWORD_ELSE);
	auto else_expr = TRY(parse_expression());

	auto e = m_cst_allocator.make<CST::TernaryExpression>();
	e->m_condition = condition;
	e->m_then_expr = then_expr;
	e->m_else_expr = else_expr;

	return make_writer(e);
}

Writer<CST::Identifier*> Parser::parse_term_identifier() {
	ErrorReport result = {{"Failed to parse identifier"}};

	Token const* token = REQUIRE_WITH(result, TokenTag::IDENTIFIER);
	auto e = m_cst_allocator.make<CST::Identifier>();
	e->m_token = token;

	return make_writer(e);
}

Writer<CST::Identifier*> Parser::parse_type_identifier() {
	ErrorReport result = {{"Failed to parse identifier"}};

	Token const* token = peek();

	if (token->m_type != TokenTag::KEYWORD_ARRAY && token->m_type != TokenTag::IDENTIFIER) {
		return make_expected_error("an identifier or 'array'", token);
	} else {
		advance_token_cursor();
	}

	auto e = m_cst_allocator.make<CST::Identifier>();
	e->m_token = token;

	return make_writer(e);
}

Writer<CST::CST*> Parser::parse_array_literal() {
	ErrorReport result = {{"Failed to parse array literal"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_ARRAY);
	REQUIRE_WITH(result, TokenTag::BRACE_OPEN);

	auto elements = TRY_WITH(result, parse_expression_list(
	    TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true));

	auto e = m_cst_allocator.make<CST::ArrayLiteral>();
	e->m_elements = std::move(elements);

	return make_writer(e);
}

Writer<CST::FuncParameters> Parser::parse_function_parameters() {

	auto open_paren = REQUIRE(TokenTag::PAREN_OPEN);

	std::vector<CST::DeclarationData> args_data;

	if (consume(TokenTag::PAREN_CLOSE))
		return make_writer(CST::FuncParameters {std::move(args_data)});

	while (1) {
		if (!match(TokenTag::IDENTIFIER))
			return make_expected_error("a parameter name", peek());

		CST::DeclarationData arg_data;

		// consume parameter name
		arg_data.m_identifier_token = peek();
		advance_token_cursor();

		// optionally consume a type hint
		if (consume(TokenTag::DECLARE)) {
			arg_data.m_type_hint = TRY(parse_type_term());
		}

		args_data.push_back(std::move(arg_data));

		if (consume(TokenTag::COMMA)) {
			// If we find a comma, we have to parse
			// another parameter, so we loop again.
			continue;
		} else if (consume(TokenTag::PAREN_CLOSE)) {
			// If we find a closing paren, we are done
			// parsing parameters, so we stop.
			break;
		} else {
			// Anything else is unexpected input, so we
			// report an error.
			return make_expected_error("',' or ')'", peek());
		}
	}

	return make_writer(CST::FuncParameters {std::move(args_data)});
}

/*
 * functions look like this:
 * fn (x : int, y, z : string) {
 *   print(x);
 * }
 */
Writer<CST::CST*> Parser::parse_function() {
	ErrorReport result = {{"Failed to parse function"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_FN);

	auto func_args = TRY(parse_function_parameters());

	if (consume(TokenTag::ARROW)) {
		auto expression = TRY_WITH(result, parse_expression());

		auto e = m_cst_allocator.make<CST::FunctionLiteral>();
		e->m_body = expression;
		e->m_args = std::move(func_args);

		return make_writer(e);
	} else if (match(TokenTag::BRACE_OPEN)) {
		auto block = TRY_WITH(result, parse_block());

		auto e = m_cst_allocator.make<CST::BlockFunctionLiteral>();
		e->m_body = block;
		e->m_args = std::move(func_args);

		return make_writer(e);
	} else {
		return make_expected_error("'=>' or '{'", peek());
	}
}

Writer<CST::Block*> Parser::parse_block() {
	ErrorReport result = {{"Failed to parse block"}};

	auto opening_brace = REQUIRE_WITH(result, TokenTag::BRACE_OPEN);

	std::vector<CST::CST*> statements;

	// loop until we find a matching closing bracket
	while (1) {

		if (match(TokenTag::END)) {
			result.add_sub_error({{"Found EOF while parsing a block"}});
			result.add_sub_error(make_located_error("(here is the opening brace)", opening_brace));
			return result;
		}

		if (consume(TokenTag::BRACE_CLOSE)) {
			break;
		}

		statements.push_back(TRY(parse_statement()));
	}

	auto e = m_cst_allocator.make<CST::Block>();
	e->m_body = std::move(statements);

	return make_writer(e);
}

Writer<CST::CST*> Parser::parse_return_statement() {
	ErrorReport result = {{"Failed to parse return statement"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_RETURN);

	auto value = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::SEMICOLON);

	auto e = m_cst_allocator.make<CST::ReturnStatement>();
	e->m_value = value;

	return make_writer(e);
}
Writer<CST::CST*> Parser::parse_if_else_stmt_or_expr() {
	ErrorReport result = {{"Failed to parse if-else statement or expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_IF);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	auto condition = TRY(parse_expression());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);

	if (match(TokenTag::KEYWORD_THEN)) {
		auto e = TRY(parse_if_else_expression(condition));
		REQUIRE_WITH(result, TokenTag::SEMICOLON);
		return make_writer(e);
	} else {

		auto body = TRY(parse_statement());

		auto e = m_cst_allocator.make<CST::IfElseStatement>();
		e->m_condition = condition;
		e->m_body = body;

		if (consume(TokenTag::KEYWORD_ELSE)) {
			e->m_else_body = TRY(parse_statement());
		}

		return make_writer(e);
	}
}

Writer<CST::CST*> Parser::parse_for_statement() {
	ErrorReport result = {{"Failed to parse for statement"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_FOR);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	// NOTE: handles semicolon already
	auto declaration = TRY_WITH(result, parse_plain_declaration_data());

	auto condition = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::SEMICOLON);

	auto action = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);

	auto body = TRY_WITH(result, parse_statement());

	auto e = m_cst_allocator.make<CST::ForStatement>();
	e->m_declaration = std::move(declaration);
	e->m_condition = condition;
	e->m_action = action;
	e->m_body = body;

	return make_writer(e);
}

Writer<CST::CST*> Parser::parse_while_statement() {
	ErrorReport result = {{"Failed to parse while statement"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_WHILE);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	auto condition = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);

	auto body = TRY_WITH(result, parse_statement());

	auto e = m_cst_allocator.make<CST::WhileStatement>();
	e->m_condition = condition;
	e->m_body = body;

	return make_writer(e);
}

Writer<CST::CST*> Parser::parse_match_expression() {
	ErrorReport result = {{"Failed to parse match expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_MATCH);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	auto matchee_and_hint = TRY_WITH(result, parse_name_and_type());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);
	REQUIRE_WITH(result, TokenTag::BRACE_OPEN);

	std::vector<CST::MatchExpression::CaseData> cases;
	while (!consume(TokenTag::BRACE_CLOSE)) {
		auto case_name = REQUIRE_WITH(result, TokenTag::IDENTIFIER);
		REQUIRE_WITH(result, TokenTag::BRACE_OPEN);

		auto name_and_type = TRY_WITH(result, parse_name_and_type());
		REQUIRE_WITH(result, TokenTag::BRACE_CLOSE);
		REQUIRE_WITH(result, TokenTag::ARROW);

		auto expression = TRY_WITH(result, parse_expression());
		REQUIRE_WITH(result, TokenTag::SEMICOLON);

		cases.push_back(
		    {case_name,
		     name_and_type.first,
		     name_and_type.second,
		     expression});
	}

	CST::Identifier matchee;
	matchee.m_token = matchee_and_hint.first;

	auto match = m_cst_allocator.make<CST::MatchExpression>();
	match->m_matchee = std::move(matchee);
	match->m_type_hint = matchee_and_hint.second;
	match->m_cases = std::move(cases);

	return make_writer(match);
}

Writer<CST::CST*> Parser::parse_sequence_expression() {
	ErrorReport result = {{"Failed to parse sequence expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_SEQ);
	auto body = TRY_WITH(result, parse_block());

	auto expr = m_cst_allocator.make<CST::SequenceExpression>();
	expr->m_body = body;

	return make_writer(expr);
}

Writer<std::pair<Token const*, CST::CST*>> Parser::parse_name_and_type(bool required_type) {
	ErrorReport result = {{"Failed to parse name and type"}};

	auto name = REQUIRE_WITH(result, TokenTag::IDENTIFIER);

	CST::CST* type = nullptr;
	if (required_type or match(TokenTag::DECLARE)) {
		REQUIRE_WITH(result, TokenTag::DECLARE);
		type = TRY_WITH(result, parse_type_term());
	}

	return make_writer<std::pair<Token const*, CST::CST*>>({name, type});
}

/*
 * Looks ahead a few tokens to predict what syntactic structure we are about to
 * parse. This prevents us from backtracking and ensures the parser runs in
 * linear time.
 */
Writer<CST::CST*> Parser::parse_statement() {
	ErrorReport result = {{"Failed to parse statement"}};

	auto* p0 = peek(0);
	if (p0->m_type == TokenTag::IDENTIFIER) {
		auto* p1 = peek(1);

		if (p1->m_type == TokenTag::DECLARE ||
		    p1->m_type == TokenTag::DECLARE_ASSIGN) {
			return parse_declaration();
		} else {
			auto expression = TRY_WITH(result, parse_full_expression());
			REQUIRE_WITH(result, TokenTag::SEMICOLON);
			return make_writer(expression);
		}
	} else if (p0->m_type == TokenTag::KEYWORD_RETURN) {
		return parse_return_statement();
	} else if (p0->m_type == TokenTag::KEYWORD_IF) {
		return parse_if_else_stmt_or_expr();
	} else if (p0->m_type == TokenTag::KEYWORD_FOR) {
		return parse_for_statement();
	} else if (p0->m_type == TokenTag::KEYWORD_WHILE) {
		return parse_while_statement();
	} else if (p0->m_type == TokenTag::BRACE_OPEN) {
		return parse_block();
	} else {
		auto expression = TRY_WITH(result, parse_full_expression());
		REQUIRE_WITH(result, TokenTag::SEMICOLON);
		return make_writer(expression);
	}

	return result;
}

Writer<std::vector<CST::CST*>> Parser::parse_type_term_arguments() {
	ErrorReport result = {{"Failed to parse type arguments"}};

	REQUIRE_WITH(result, TokenTag::POLY_OPEN);

	std::vector<CST::CST*> args;

	while (!consume(TokenTag::POLY_CLOSE)) {
		args.push_back(TRY_WITH(result, parse_type_term()));
	}

	return make_writer(std::move(args));
}

Writer<CST::CST*> Parser::parse_type_term() {
	ErrorReport result = {{"Failed to parse type"}};

	auto callee = TRY_WITH(result, parse_type_identifier());

	if (!match(TokenTag::POLY_OPEN))
		return make_writer(callee);

	auto args = TRY_WITH(result, parse_type_term_arguments());

	auto e = m_cst_allocator.make<CST::TypeTerm>();
	e->m_callee = callee;
	e->m_args = std::move(args);
	return make_writer(e);
}

Writer<std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>> Parser::parse_type_list(
    bool with_identifiers = false) {
	ErrorReport result = {{"Failed to parse type list"}};

	std::vector<CST::Identifier> identifiers;
	std::vector<CST::CST*> types;

	REQUIRE_WITH(result, TokenTag::BRACE_OPEN);

	while(!consume(TokenTag::BRACE_CLOSE)) {
		if (with_identifiers) {
			auto cons = TRY_WITH(result, parse_term_identifier());
			REQUIRE_WITH(result, TokenTag::DECLARE);

			identifiers.push_back(std::move(*cons));
		}

		auto type = TRY_WITH(result, parse_type_term());
		REQUIRE_WITH(result, TokenTag::SEMICOLON);

		types.push_back(type);
	}

	return make_writer<
	    std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>>(
	    make_pair(std::move(identifiers), std::move(types)));
}

Writer<CST::CST*> Parser::parse_type_var() {
	ErrorReport result = {{"Failed to parse type var"}};

	REQUIRE_WITH(result, TokenTag::AT);

	auto token = REQUIRE_WITH(result, TokenTag::IDENTIFIER);

	auto t = m_cst_allocator.make<CST::TypeVar>();
	t->m_token = token;

	return make_writer(t);
}

Writer<CST::CST*> Parser::parse_type_function() {
	ErrorReport result = {{"Failed to parse type function"}};

	if (consume(TokenTag::KEYWORD_UNION)) {
		auto tl = TRY_WITH(result, parse_type_list(true));

		auto u = m_cst_allocator.make<CST::UnionExpression>();
		u->m_constructors = std::move(tl.first);
		u->m_types = std::move(tl.second);

		return make_writer(u);
	} else if (consume(TokenTag::KEYWORD_STRUCT)) {
		auto tl = TRY_WITH(result, parse_type_list(true));

		auto s = m_cst_allocator.make<CST::StructExpression>();
		s->m_fields = std::move(tl.first);
		s->m_types = std::move(tl.second);

		return make_writer(s);
	}

	return result;
}


ParserResult parse_program(LexerResult lexer_result, CST::Allocator& allocator) {
	Parser p {lexer_result.tokens, lexer_result.file_context, allocator};
	Writer<CST::CST*> w = p.parse_top_level();
	return {w.m_result, std::move(w.m_error), std::move(lexer_result.tokens), std::move(lexer_result.file_context)};
}

ParserResult parse_expression(LexerResult lexer_result, CST::Allocator& allocator) {
	Parser p {lexer_result.tokens, lexer_result.file_context, allocator};
	Writer<CST::CST*> w = p.parse_expression();
	return {w.m_result, std::move(w.m_error), std::move(lexer_result.tokens), std::move(lexer_result.file_context)};
}
