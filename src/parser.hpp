#include "lexer.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ast.hpp"

#include <cassert>
#include <cstdlib>

struct ParseError {
	std::string m_text;
	std::vector<ParseError> m_sub_errors;

	bool ok () {
		return m_sub_errors.empty() && m_text.empty();
	}

	void print (int d = 1) {
		for(int i = 0; i < d; ++i)
			std::cerr << '-';
		std::cerr << ' ';
		std::cerr << m_text << '\n';
		for(auto& sub : m_sub_errors){
			sub.print(d+1);
		}
	}
};

template<typename T>
struct Writer {
	ParseError m_error {};
	T m_result {};

	bool ok () {
		return m_error.ok();
	}
};

template<typename T>
Writer<T> make_writer (T x) {
	return {{}, std::move(x)};
}

struct Parser {
	Lexer* m_lexer;

	Writer<std::unique_ptr<AST>> parse_top_level ();
	Writer<std::unique_ptr<AST>> parse_declaration ();
	Writer<std::unique_ptr<AST>> parse_expression ();
	Writer<std::unique_ptr<AST>> parse_function ();
	Writer<std::unique_ptr<AST>> parse_block ();

	Writer<Token const*> require (token_type t);
};

