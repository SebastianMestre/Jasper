#pragma once

#include <vector>

#include "./algorithms/trie.hpp"
#include "token.hpp"
#include "token_array.hpp"
#include "token_tag.hpp"

struct Lexer {
	Lexer(std::vector<char>, TokenArray&);

	char char_at(int index);
	char current_char() {
		return char_at(m_source_index);
	}
	char next_char() {
		return char_at(m_source_index + 1);
	}
	char peek_char(int di = 0) {
		return char_at(m_source_index + di);
	}

	bool done() {
		return m_done;
	}

	bool consume_symbol();
	bool consume_string();
	bool consume_comment();
	bool consume_identifier_or_keyword();
	bool consume_number();
	void consume_token();
	void push_token(TokenTag, int);

	std::pair<bool, TokenTag> is_keyword(InternedString const&);

	void advance();
	void regress();

	Token const& token_at(int index);
	Token const& current_token() {
		return token_at(m_token_index);
	}
	Token const& next_token() {
		return token_at(m_token_index + 1);
	}
	Token const& peek_token(int dt = 0) {
		return token_at(m_token_index + dt);
	}

	std::vector<char> m_source;
	TokenArray& m_tokens;

	Trie m_symbols_trie;

	bool m_done {false};

	int m_source_index {0};
	int m_token_index {0};

	int m_current_line {0};
	int m_current_column {0};
};
