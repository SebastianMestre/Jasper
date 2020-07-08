#pragma once

// TODO: do not include iostream here
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

struct ParseError {
	std::string m_text;
	std::vector<ParseError> m_sub_errors;

	bool ok() { return m_sub_errors.empty() && m_text.empty(); }

	void print(int d = 1) {
		/* indent */
		for (int i = 0; i < d; ++i)
			std::cerr << '-';

		/* print error */
		std::cerr << ' ';
		std::cerr << m_text << '\n';

		/* print suberrors one place further right */
		for (auto& sub : m_sub_errors) {
			sub.print(d + 1);
		}
	}
};

template <typename T>
struct Writer {
	ParseError m_error{};
	T m_result{};

	bool ok() { return m_error.ok(); }
};

template <typename T>
Writer<T> make_writer(T x) {
	return { {}, std::move(x) };
}

struct Parser {
	/* token handler */
	Lexer* m_lexer;

	Writer<std::vector<std::unique_ptr<AST::AST>>> parse_declaration_list(token_type);
	Writer<std::vector<std::unique_ptr<AST::AST>>> parse_expression_list(token_type, token_type, bool);

	Writer<std::unique_ptr<AST::AST>> parse_top_level();
	Writer<std::unique_ptr<AST::AST>> parse_identifier();
	Writer<std::unique_ptr<AST::AST>> parse_declaration();
	Writer<std::unique_ptr<AST::AST>> parse_expression(int bp = 0);
	Writer<std::unique_ptr<AST::AST>> parse_terminal();
	Writer<std::unique_ptr<AST::AST>> parse_function();
	Writer<std::unique_ptr<AST::AST>> parse_object_literal();
	Writer<std::unique_ptr<AST::AST>> parse_array_literal();
	Writer<std::unique_ptr<AST::AST>> parse_dictionary_literal();
	Writer<std::vector<std::unique_ptr<AST::AST>>> parse_argument_list();
	Writer<std::unique_ptr<AST::AST>> parse_block();
	Writer<std::unique_ptr<AST::AST>> parse_statement();
	Writer<std::unique_ptr<AST::AST>> parse_return_statement();
	Writer<std::unique_ptr<AST::AST>> parse_if_statement();
	Writer<std::unique_ptr<AST::AST>> parse_for_statement();
	Writer<std::unique_ptr<AST::AST>> parse_type_term();

	Writer<Token const*> require(token_type t);

	Token const* peek(int dt = 0) { return &m_lexer->peek_token(dt); }
};

