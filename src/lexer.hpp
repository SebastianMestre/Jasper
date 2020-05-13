#pragma once

#include <vector>

#include "token.hpp"
#include "token_type.hpp"

struct TokenArray;

/**
 * Converts raw bytes to tokens.
 *
 * Ensures reference stability for the Tokens it
 * hands out.
 */
struct Lexer {
	std::vector<char> m_source;
	TokenArray& m_tokens;

	int m_source_index{ 0 };
	int m_token_index{ 0 };

	int m_current_line{ 0 };
	int m_current_column{ 0 };

	char char_at(int index);
	char current_char() { return char_at(m_source_index); }
	char next_char() { return char_at(m_source_index + 1); }
	char peek_char(int di = 0) { return char_at(m_source_index + di); }

	bool done() { return current_char() == '\0'; }

	void consume_token();
	bool consume_keyword();
	void push_token(token_type, int);

	void advance();
	void regress();

	Token const& token_at(int index);
	Token const& current_token() { return token_at(m_token_index); }
	Token const& next_token() { return token_at(m_token_index + 1); }
	Token const& peek_token(int dt = 0) { return token_at(m_token_index + dt); }

};
