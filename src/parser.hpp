#pragma once

#include <vector>

#include "error_report.hpp"
#include "lexer.hpp"

namespace AST {
struct AST;
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
	AST::Allocator* m_ast_allocator;

	Writer<std::vector<AST::Declaration>> parse_declaration_list(TokenTag);
	Writer<std::vector<AST::AST*>>
	parse_expression_list(TokenTag, TokenTag, bool);

	Writer<AST::AST*> parse_top_level();

	Writer<AST::AST*> parse_sequence_expression();
	Writer<AST::Identifier*> parse_identifier(bool types_allowed = false);
	Writer<AST::Declaration*> parse_declaration();
	Writer<AST::AST*> parse_expression(int bp = 0);
	Writer<AST::AST*> parse_terminal();
	Writer<AST::AST*> parse_ternary_expression();
	Writer<AST::AST*> parse_function();
	Writer<AST::AST*> parse_object_literal();
	Writer<AST::AST*> parse_array_literal();
	Writer<AST::AST*> parse_dictionary_literal();
	Writer<std::vector<AST::AST*>> parse_argument_list();
	Writer<AST::AST*> parse_block();
	Writer<AST::AST*> parse_statement();
	Writer<AST::AST*> parse_return_statement();
	Writer<AST::AST*> parse_if_else_statement();
	Writer<AST::AST*> parse_for_statement();
	Writer<AST::AST*> parse_while_statement();
	Writer<AST::AST*> parse_match_expression();
	Writer<std::pair<Token const*, AST::AST*>> parse_name_and_type(
	    bool required_type = false);
	Writer<AST::AST*> parse_type_term();
	Writer<std::vector<AST::AST*>> parse_type_term_arguments();
	Writer<std::pair<std::vector<AST::Identifier>, std::vector<AST::AST*>>> parse_type_list(
	    bool);
	Writer<AST::AST*> parse_type_var();
	Writer<AST::AST*> parse_type_function();

	Writer<Token const*> require(TokenTag);
	bool consume(TokenTag);
	bool match(TokenTag);

	Token const* peek(int dt = 0) {
		return &m_lexer->peek_token(dt);
	}
};
