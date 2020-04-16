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
		/* indent */
		for(int i = 0; i < d; ++i)
			std::cerr << '-';
		
		/* print error */
		std::cerr << ' ';
		std::cerr << m_text << '\n';

		/* print suberrors one place further right */
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
	/* token handler */
	Lexer* m_lexer;

	Writer<std::unique_ptr<AST>> parse_top_level ();
	Writer<std::unique_ptr<AST>> parse_declaration ();
	Writer<std::unique_ptr<AST>> parse_expression ();

	/* checks the existence of token t */
	Writer<Token const*> require (token_type t);
};

