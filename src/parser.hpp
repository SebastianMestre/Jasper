#pragma once

#include <vector>

#include "error_report.hpp"
#include "lexer.hpp"

namespace CST {
struct CST;
struct Allocator;
struct Identifier;
struct Declaration;
}

template <typename T>
struct Writer {
	ErrorReport m_error {};
	T m_result {};

	bool ok() {
		return m_error.ok();
	}
};

template <typename T>
Writer<T> make_writer(T x) {
	return {{}, std::move(x)};
}

struct Parser {
	/* token handler */
	Lexer* m_lexer;
	CST::Allocator* m_ast_allocator;

	Writer<std::vector<CST::Declaration>> parse_declaration_list(TokenTag);
	Writer<std::vector<CST::CST*>>
	parse_expression_list(TokenTag, TokenTag, bool);

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
	Writer<std::pair<Token const*, CST::CST*>> parse_name_and_type(
	    bool required_type = false);
	Writer<CST::CST*> parse_type_term();
	Writer<std::vector<CST::CST*>> parse_type_term_arguments();
	Writer<std::pair<std::vector<CST::Identifier>, std::vector<CST::CST*>>> parse_type_list(
	    bool);
	Writer<CST::CST*> parse_type_var();
	Writer<CST::CST*> parse_type_function();

	Writer<Token const*> require(TokenTag);
	bool consume(TokenTag);
	bool match(TokenTag);

	Token const* peek(int dt = 0) {
		return &m_lexer->peek_token(dt);
	}
};
