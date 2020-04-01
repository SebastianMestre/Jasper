#pragma once

#include <vector>
#include <string>

#include <cassert>
#include <cctype>

inline const char* token_type_string[] = {
	"+",
	"-",
	"/",
	"*",
	"&",
	"|",
	"^",

	"LT",
	"GT",
	"LTE",
	"GTE",

	"COMPL",

	"LOGIC_COMPL",
	"LOGIC_AND",
	"LOGIC_IOR",

	"EQUAL",
	"NOT_EQUAL",

	"=",
	":=",
	":",

	"INCREMENT",
	"DECREMENT",

	"ADD_TO",
	"SUB_TO",
	"MUL_TO",
	"DIV_TO",
	"AND_TO",
	"IOR_TO",
	"XOR_TO",

	"BRACE_OPEN",
	"BRACKET_OPEN",
	"PAREN_OPEN",

	"BRACE_CLOSE",
	"BRACKET_CLOSE",
	"PAREN_CLOSE",

	"SEMICOLON",
	"COLON",

	"NUMBER",
	"IDENTIFIER",
	"KEYWORD",
};

enum class token_type {
	ADD,
	SUB,
	DIV,
	MUL,
	AND,
	IOR,
	XOR,

	LT,
	GT,
	LTE,
	GTE,

	COMPL,

	LOGIC_COMPL,
	LOGIC_AND,
	LOGIC_IOR,

	EQUAL,
	NOT_EQUAL,

	ASSIGN,
	DECLARE_ASSIGN,
	DECLARE,

	INCREMENT,
	DECREMENT,

	ADD_TO,
	SUB_TO,
	MUL_TO,
	DIV_TO,
	AND_TO,
	IOR_TO,
	XOR_TO,

	BRACE_OPEN,
	BRACKET_OPEN,
	PAREN_OPEN,

	BRACE_CLOSE,
	BRACKET_CLOSE,
	PAREN_CLOSE,

	SEMICOLON,
	COLON,

	NUMBER,
	IDENTIFIER,
	KEYWORD,
};

struct Token {
	token_type m_type;
	std::string m_text;
	int m_line0, m_col0;
	int m_line1, m_col1;
};

struct TokenArray {
	static constexpr int bucket_size = 16;
	std::vector<std::vector<Token>> m_buckets;

	void push_back (Token t) {
		if (m_buckets.empty() || m_buckets.back().size() == bucket_size) {
			m_buckets.push_back(std::vector<Token>{});
			m_buckets.back().reserve(bucket_size);
		}
		m_buckets.back().push_back(std::move(t));
	}

	Token& back() {
		return m_buckets.back().back();
	}

	Token& at(int i) {
		return m_buckets[i/bucket_size][i%bucket_size];
	}

	int size () {
		return m_buckets.empty() ? 0 : (int(m_buckets.size())-1) * bucket_size + int(m_buckets.back().size());
	}
};

struct Lexer {
	std::vector<char> m_source;
	TokenArray m_tokens;

	int m_source_index {0};
	int m_token_index {0};

	int m_current_line {0};
	int m_current_column {0};

	char char_at(int index);
	char current_char () { return char_at(m_source_index); }
	char next_char () { return char_at(m_source_index+1); }

	bool done () { return current_char() == '\0'; }

	void consume_token ();
	void push_token (token_type, int);

	void advance ();
	void regress ();

	Token const& token_at (int index);
	Token const& current_token () { return token_at(m_token_index); }
	Token const& next_token () { return token_at(m_token_index+1); }
};
