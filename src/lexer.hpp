#pragma once

#include <string>
#include <vector>

#include "token.hpp"
#include "token_type.hpp"

/**
 * Bucket-like token list
 * that avoids reallocations,
 * to preserve references of tokens
 * given to other entities.
 */
struct TokenArray {
	static constexpr int bucket_size = 16;
	std::vector<std::vector<Token>> m_buckets;

	void push_back(Token t) {
		if (m_buckets.empty() || m_buckets.back().size() == bucket_size) {
			m_buckets.push_back(std::vector<Token>{});
			m_buckets.back().reserve(bucket_size);
		}
		m_buckets.back().push_back(std::move(t));
	}

	Token& back() { return m_buckets.back().back(); }

	Token& at(int i) { return m_buckets[i / bucket_size][i % bucket_size]; }

	int size() {
		return m_buckets.empty() ? 0
		                         : (int(m_buckets.size()) - 1) * bucket_size
		        + int(m_buckets.back().size());
	}
};

/**
 * Converts raw bytes to tokens.
 *
 * Ensures reference stability for the Tokens it
 * hands out.
 */
struct Lexer {
	std::vector<char> m_source;
	TokenArray m_tokens;

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
