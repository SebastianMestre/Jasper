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

	Writer<CST::Stmt*> parse_statement();
	Writer<CST::Block*> parse_block();
	Writer<CST::ReturnStatement*> parse_return_statement();
	Writer<CST::Stmt*> parse_if_else_stmt_or_expr();
	Writer<CST::ForStatement*> parse_for_statement();
	Writer<CST::WhileStatement*> parse_while_statement();

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

	template<typename T, typename...Args>
	T* make(Args&& ...args) {
		return m_cst_allocator.make<T>(std::forward<Args>(args)...);
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

	return make_writer(make<CST::Program>(std::move(declarations)));
}

Writer<std::vector<CST::CST*>> Parser::parse_expression_list(
    TokenTag delimiter, TokenTag terminator, bool allow_trailing_delimiter) {

	std::vector<CST::CST*> expressions;

	if (consume(terminator))
		return make_writer(std::move(expressions));

	while (1) {

		expressions.push_back(TRY(parse_expression()));

		auto p0 = peek();

		if (consume(delimiter)) {

			if (match(terminator)) {
				if (allow_trailing_delimiter) {
					advance_token_cursor();
					break;
				}

				return make_located_error(
				    "Found trailing delimiter in expression list", p0);
			}
		} else if (consume(terminator)) {
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

		return make_writer(make<CST::FuncDeclaration>(identifier, std::move(args), expression));
	}

	if (match(TokenTag::BRACE_OPEN)) {
		auto block = TRY_WITH(result, parse_block());

		REQUIRE_WITH(result, TokenTag::SEMICOLON);

		return make_writer(make<CST::BlockFuncDeclaration>(identifier, std::move(args), block));
	}

	result.add_sub_error(make_expected_error("'=>' or '{'", peek()));
	return result;
}

Writer<CST::PlainDeclaration*> Parser::parse_plain_declaration() {
	ErrorReport result = {{"Failed to parse declaration"}};

	auto decl_data = TRY_WITH(result, parse_plain_declaration_data());

	return make_writer(make<CST::PlainDeclaration>(std::move(decl_data)));
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

		if (match(TokenTag::SEMICOLON) ||
		    match(TokenTag::END) ||
		    match(TokenTag::BRACE_CLOSE) ||
		    match(TokenTag::BRACKET_CLOSE) ||
		    match(TokenTag::PAREN_CLOSE) ||
		    match(TokenTag::COMMA) ||
		    match(TokenTag::KEYWORD_THEN) ||
		    match(TokenTag::KEYWORD_ELSE)) {
			break;
		}

		if (not is_binary_operator(op->m_type))
			return make_expected_error("a binary operator", op);

		auto op_bp = binding_power_of(op->m_type);
		auto& lp = op_bp.left;
		auto& rp = op_bp.right;

		if (lp < bp)
			break;

		if (match(TokenTag::PAREN_OPEN)) {
			lhs = make<CST::CallExpression>(lhs, TRY(parse_argument_list()));
			continue;
		}

		if (consume(TokenTag::BRACKET_OPEN)) {

			auto index = TRY(parse_expression());

			REQUIRE(TokenTag::BRACKET_CLOSE);

			lhs = make<CST::IndexExpression>(lhs, index);
			continue;
		}

		if (match(TokenTag::POLY_OPEN)) {
			lhs = make<CST::TypeTerm>(lhs, TRY(parse_type_term_arguments()));
			continue;
		}

		if (consume(TokenTag::DOT)) {
			lhs = make<CST::AccessExpression>(lhs, REQUIRE(TokenTag::IDENTIFIER));
			continue;
		}

		if (consume(TokenTag::BRACE_OPEN)) {
			auto args = TRY(parse_expression_list(
			    TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true));

			lhs = make<CST::ConstructorExpression>(lhs, std::move(args));
			continue;
		}

		advance_token_cursor();
		auto rhs = TRY(parse_expression(rp));

		lhs = make<CST::BinaryExpression>(op, lhs, rhs);
	}

	return make_writer(lhs);
}

/* We say a terminal is any expression that is not an infix expression.
 * This is not what the term usually means in the literature.
 */
Writer<CST::CST*> Parser::parse_terminal() {
	auto token = peek();

	if (consume(TokenTag::KEYWORD_NULL)) {
		return make_writer(make<CST::NullLiteral>());
	}

	if (consume(TokenTag::KEYWORD_TRUE)) {
		return make_writer(make<CST::BooleanLiteral>(token));
	}

	if (consume(TokenTag::KEYWORD_FALSE)) {
		return make_writer(make<CST::BooleanLiteral>(token));
	}

	if (consume(TokenTag::SUB) || consume(TokenTag::ADD)) {

		// NOTE: we store the sign token of the source code for future
		// feature of printing the source code when an error occurs
		auto value = peek();

		bool is_negative = token->m_type == TokenTag::SUB;
		if (consume(TokenTag::INTEGER)) {
			return make_writer(make<CST::IntegerLiteral>(is_negative, token, value));
		} else if (consume(TokenTag::NUMBER)) {
			return make_writer(make<CST::NumberLiteral>(is_negative, token, value));
		}

		return is_negative
			? make_located_error("Stray minus sign with no number", token)
			: make_located_error("Stray plus sign with no number", token);
	}

	if (consume(TokenTag::INTEGER)) {
		return make_writer(make<CST::IntegerLiteral>(false, nullptr, token));
	}

	if (consume(TokenTag::NUMBER)) {
		return make_writer(make<CST::NumberLiteral>(false, nullptr, token));
	}

	if (consume(TokenTag::IDENTIFIER)) {
		return make_writer(make<CST::Identifier>(token));
	}

	if (consume(TokenTag::STRING)) {
		return make_writer(make<CST::StringLiteral>(token));
	}

	if (match(TokenTag::KEYWORD_FN)) {
		return parse_function();
	}

	if (match(TokenTag::KEYWORD_IF)) {
		return parse_if_else_expression();
	}

	// parse a parenthesized expression.
	if (consume(TokenTag::PAREN_OPEN)) {
		auto expr = TRY(parse_expression());
		REQUIRE(TokenTag::PAREN_CLOSE);
		return make_writer(expr);
	}

	if (match(TokenTag::KEYWORD_ARRAY)) {
		return parse_array_literal();
	}

	if (match(TokenTag::KEYWORD_UNION) ||
	    match(TokenTag::KEYWORD_STRUCT)) {
		// TODO: do the other type functions
		return parse_type_function();
	}

	if (match(TokenTag::KEYWORD_MATCH)) {
		return parse_match_expression();
	}

	if (match(TokenTag::KEYWORD_SEQ)) {
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

	return make_writer(make<CST::TernaryExpression>(condition, then_expr, else_expr));
}

Writer<CST::Identifier*> Parser::parse_term_identifier() {
	ErrorReport result = {{"Failed to parse identifier"}};

	Token const* token = REQUIRE_WITH(result, TokenTag::IDENTIFIER);

	return make_writer(make<CST::Identifier>(token));
}

Writer<CST::Identifier*> Parser::parse_type_identifier() {
	ErrorReport result = {{"Failed to parse identifier"}};

	Token const* token = peek();

	if (token->m_type != TokenTag::KEYWORD_ARRAY && token->m_type != TokenTag::IDENTIFIER) {
		return make_expected_error("an identifier or 'array'", token);
	} else {
		advance_token_cursor();
	}

	return make_writer(make<CST::Identifier>(token));
}

Writer<CST::CST*> Parser::parse_array_literal() {
	ErrorReport result = {{"Failed to parse array literal"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_ARRAY);
	REQUIRE_WITH(result, TokenTag::BRACE_OPEN);

	auto elements = TRY_WITH(result, parse_expression_list(
	    TokenTag::SEMICOLON, TokenTag::BRACE_CLOSE, true));

	return make_writer(make<CST::ArrayLiteral>(std::move(elements)));
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
		return make_writer(make<CST::FunctionLiteral>(expression, std::move(func_args)));
	} else if (match(TokenTag::BRACE_OPEN)) {
		auto block = TRY_WITH(result, parse_block());
		return make_writer(make<CST::BlockFunctionLiteral>(block, std::move(func_args)));
	} else {
		return make_expected_error("'=>' or '{'", peek());
	}
}


Writer<CST::Stmt*> Parser::parse_statement() {
	ErrorReport result = {{"Failed to parse statement"}};

	if (match(TokenTag::IDENTIFIER) &&
	    (peek(1)->m_type == TokenTag::DECLARE ||
	     peek(1)->m_type == TokenTag::DECLARE_ASSIGN)) {
		return parse_declaration();
	} else if (match(TokenTag::KEYWORD_RETURN)) {
		return parse_return_statement();
	} else if (match(TokenTag::KEYWORD_IF)) {
		return parse_if_else_stmt_or_expr();
	} else if (match(TokenTag::KEYWORD_FOR)) {
		return parse_for_statement();
	} else if (match(TokenTag::KEYWORD_WHILE)) {
		return parse_while_statement();
	} else if (match(TokenTag::BRACE_OPEN)) {
		return parse_block();
	}

	auto expression = TRY_WITH(result, parse_full_expression());
	REQUIRE_WITH(result, TokenTag::SEMICOLON);
	return make_writer(make<CST::ExpressionStatement>(expression));
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

	return make_writer(make<CST::Block>(std::move(statements)));
}

Writer<CST::ReturnStatement*> Parser::parse_return_statement() {
	ErrorReport result = {{"Failed to parse return statement"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_RETURN);

	auto value = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::SEMICOLON);

	return make_writer(make<CST::ReturnStatement>(value));
}

Writer<CST::Stmt*> Parser::parse_if_else_stmt_or_expr() {
	ErrorReport result = {{"Failed to parse if-else statement or expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_IF);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	auto condition = TRY(parse_expression());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);

	if (match(TokenTag::KEYWORD_THEN)) {
		auto e = TRY(parse_if_else_expression(condition));
		REQUIRE_WITH(result, TokenTag::SEMICOLON);
		return make_writer(make<CST::ExpressionStatement>(e));
	} else {

		auto body = TRY(parse_statement());

		CST::CST* else_body = nullptr;
		if (consume(TokenTag::KEYWORD_ELSE))
			else_body = TRY(parse_statement());

		return make_writer(make<CST::IfElseStatement>(condition, body, else_body));
	}
}

Writer<CST::ForStatement*> Parser::parse_for_statement() {
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

	return make_writer(make<CST::ForStatement>(std::move(declaration), condition, action, body));
}

Writer<CST::WhileStatement*> Parser::parse_while_statement() {
	ErrorReport result = {{"Failed to parse while statement"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_WHILE);
	REQUIRE_WITH(result, TokenTag::PAREN_OPEN);

	auto condition = TRY_WITH(result, parse_expression());
	REQUIRE_WITH(result, TokenTag::PAREN_CLOSE);

	auto body = TRY_WITH(result, parse_statement());

	return make_writer(make<CST::WhileStatement>(condition, body));
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

		auto expression = TRY(parse_expression());
		REQUIRE_WITH(result, TokenTag::SEMICOLON);

		cases.push_back({
			case_name,
			name_and_type.first,
			name_and_type.second,
			expression});
	}

	auto matchee = std::move(matchee_and_hint.first);
	auto hint = matchee_and_hint.second;

	return make_writer(make<CST::MatchExpression>(std::move(matchee), hint, std::move(cases)));
}

Writer<CST::CST*> Parser::parse_sequence_expression() {
	ErrorReport result = {{"Failed to parse sequence expression"}};

	REQUIRE_WITH(result, TokenTag::KEYWORD_SEQ);
	auto body = TRY_WITH(result, parse_block());

	return make_writer(make<CST::SequenceExpression>(body));
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

	return make_writer(make<CST::TypeTerm>(callee, std::move(args)));
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

	return make_writer(make<CST::TypeVar>(token));
}

Writer<CST::CST*> Parser::parse_type_function() {
	ErrorReport result = {{"Failed to parse type function"}};

	if (consume(TokenTag::KEYWORD_UNION)) {
		auto tl = TRY_WITH(result, parse_type_list(true));
		return make_writer(make<CST::UnionExpression>(std::move(tl.first), std::move(tl.second)));
	} else if (consume(TokenTag::KEYWORD_STRUCT)) {
		auto tl = TRY_WITH(result, parse_type_list(true));
		return make_writer(make<CST::StructExpression>(std::move(tl.first), std::move(tl.second)));
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
