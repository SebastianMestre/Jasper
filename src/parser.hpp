#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "error_report.hpp"
#include "lexer.hpp"

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

	Writer<std::vector<std::unique_ptr<AST::AST>>> parse_declaration_list(TokenTag);
	Writer<std::vector<std::unique_ptr<AST::AST>>>
	parse_expression_list(TokenTag, TokenTag, bool);

	Writer<std::unique_ptr<AST::AST>> parse_top_level();
	Writer<std::unique_ptr<AST::Identifier>> parse_identifier();
	Writer<std::unique_ptr<AST::AST>> parse_declaration();
	Writer<std::unique_ptr<AST::AST>> parse_expression(int bp = 0);
	Writer<std::unique_ptr<AST::AST>> parse_terminal();
	Writer<std::unique_ptr<AST::AST>> parse_ternary_expression();
	Writer<std::unique_ptr<AST::AST>> parse_function();
	Writer<std::unique_ptr<AST::AST>> parse_object_literal();
	Writer<std::unique_ptr<AST::AST>> parse_array_literal();
	Writer<std::unique_ptr<AST::AST>> parse_dictionary_literal();
	Writer<std::vector<std::unique_ptr<AST::AST>>> parse_argument_list();
	Writer<std::unique_ptr<AST::AST>> parse_block();
	Writer<std::unique_ptr<AST::AST>> parse_statement();
	Writer<std::unique_ptr<AST::AST>> parse_return_statement();
	Writer<std::unique_ptr<AST::AST>> parse_if_else_statement();
	Writer<std::unique_ptr<AST::AST>> parse_for_statement();
	Writer<std::unique_ptr<AST::AST>> parse_while_statement();
	Writer<std::unique_ptr<AST::AST>> parse_type_term();
	Writer<std::pair<std::vector<AST::Identifier>,std::vector<std::unique_ptr<AST::AST>>>>
	    parse_type_list(bool);
	Writer<std::unique_ptr<AST::AST>> parse_type_var();
	Writer<std::unique_ptr<AST::AST>> parse_type_function();

	Writer<Token const*> require(TokenTag);
	bool consume(TokenTag);
	bool match(TokenTag);

	Token const* peek(int dt = 0) {
		return &m_lexer->peek_token(dt);
	}
};
