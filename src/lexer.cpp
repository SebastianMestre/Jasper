#include "lexer.hpp"

#include <vector>

#include <cassert>
#include <cctype>
#include <cstdlib>

#include "token_array.hpp"

bool is_identifier_start_char(char c) {
	return isalpha(c) || c == '_';
}

bool is_identifier_char(char c) {
	return isalpha(c) || c == '_' || isdigit(c);
}

char Lexer::char_at(int index) {
	return index < int(m_source.size()) ? m_source[index] : '\0';
}

void Lexer::push_token(TokenTag tt, int width) {

	char const* base_ptr = m_source.data() + m_source_index;

	Token t = {
	    tt,
	    InternedString(base_ptr, width),
	    m_current_line,
	    m_current_column,
	    m_current_line,
	    m_current_column + width};

	m_tokens.push_back(t);

	m_source_index += width;
	m_current_column += width;
}

bool Lexer::consume_identifier_or_keyword() {
	if (!is_identifier_start_char(current_char()))
		return false;

	char const* base_ptr = m_source.data() + m_source_index;
	size_t len = 1;

	while (is_identifier_char(next_char())) {
		len += 1;
		m_source_index += 1;
	}

	m_source_index += 1;
	m_current_column += len;

	auto tag = TokenTag::IDENTIFIER;
	auto text = InternedString {base_ptr, len};

	auto keyword_lookup = is_keyword(text);
	if (keyword_lookup.first)
		tag = keyword_lookup.second;

	m_tokens.push_back(
	    {tag,
	     std::move(text),
	     m_current_line,
	     m_current_column - int(len),
	     m_current_line,
	     m_current_column});

	return true;
}

std::pair<bool, TokenTag> Lexer::is_keyword(InternedString const& str) {
	// sorted by commonness (off the top of my head, probably inaccurate)
	static std::pair<InternedString, TokenTag> const keywords[] = {
		{{"if"}, TokenTag::KEYWORD_IF},
		{{"for"}, TokenTag::KEYWORD_FOR},
		{{"else"}, TokenTag::KEYWORD_ELSE},
		{{"fn"}, TokenTag::KEYWORD_FN},
		{{"then"}, TokenTag::KEYWORD_THEN},
		{{"return"}, TokenTag::KEYWORD_RETURN},
		{{"while"}, TokenTag::KEYWORD_WHILE},
		{{"match"}, TokenTag::KEYWORD_MATCH},
		{{"true"}, TokenTag::KEYWORD_TRUE},
		{{"false"}, TokenTag::KEYWORD_FALSE},
		{{"array"}, TokenTag::KEYWORD_ARRAY},
		{{"dict"}, TokenTag::KEYWORD_DICT},
		{{"null"}, TokenTag::KEYWORD_NULL},
		{{"seq"}, TokenTag::KEYWORD_SEQ},
		{{"obt"}, TokenTag::KEYWORD_OBJECT},
		{{"tuple"}, TokenTag::KEYWORD_TUPLE},
		{{"struct"}, TokenTag::KEYWORD_STRUCT},
		{{"union"}, TokenTag::KEYWORD_UNION},
	};

	// The strings are interned, so the check is an O(1) pointer comparison.
	// Still, it can be sped up by splitting based on first character or length.
	for (auto const& keyword : keywords)
		if (keyword.first == str)
			return {true, keyword.second};

	return {false, TokenTag::END};
}

void Lexer::consume_token() {

	switch (current_char()) {

	case '+':
		switch (next_char()) {
		case '+':
			push_token(TokenTag::INCREMENT, 2);
			break;
		case '=':
			push_token(TokenTag::ADD_TO, 2);
			break;
		default:
			push_token(TokenTag::ADD, 1);
		}
		break;

	case '-':
		switch (next_char()) {
		case '-':
			push_token(TokenTag::DECREMENT, 2);
			break;
		case '=':
			push_token(TokenTag::SUB_TO, 2);
			break;
		default:
			push_token(TokenTag::SUB, 1);
		}
		break;

	case '*':
		switch (next_char()) {
		case '=':
			push_token(TokenTag::MUL_TO, 2);
			break;
		default:
			push_token(TokenTag::MUL, 1);
		}
		break;

	case '/':
		switch (next_char()) {
		case '/':
			assert(consume_comment());
			break;
		case '=':
			push_token(TokenTag::DIV_TO, 2);
			break;
		default:
			push_token(TokenTag::DIV, 1);
		}
		break;

	case '&':
		switch (next_char()) {
		case '&':
			push_token(TokenTag::LOGIC_AND, 2);
			break;
		case '=':
			push_token(TokenTag::AND_TO, 2);
			break;
		default:
			push_token(TokenTag::AND, 1);
		}
		break;

	case '|':
		switch (next_char()) {
		case '|':
			push_token(TokenTag::LOGIC_IOR, 2);
			break;
		case '>':
			push_token(TokenTag::PIZZA, 2);
			break;
		case '=':
			push_token(TokenTag::IOR_TO, 2);
			break;
		default:
			push_token(TokenTag::IOR, 1);
		}
		break;

	case '^':
		switch (next_char()) {
		/* case '^': push_token(TokenTag::LOGIC_XOR, 2); break; */
		case '=':
			push_token(TokenTag::XOR_TO, 2);
			break;
		default:
			push_token(TokenTag::XOR, 1);
		}
		break;

	case ':':
		switch (next_char()) {
		case '=':
			push_token(TokenTag::DECLARE_ASSIGN, 2);
			break;
		default:
			push_token(TokenTag::DECLARE, 1);
		}
		break;

	case '=':
		switch (next_char()) {
		case '=':
			push_token(TokenTag::EQUAL, 2);
			break;
		case '>':
			push_token(TokenTag::ARROW, 2);
			break;
		default:
			push_token(TokenTag::ASSIGN, 1);
		}
		break;

	case '<':
		switch (next_char()) {
		case '=':
			push_token(TokenTag::LTE, 2);
			break;
		default:
			push_token(TokenTag::LT, 1);
		}
		break;

	case '>':
		switch (next_char()) {
		case ')':
			push_token(TokenTag::POLY_CLOSE, 2);
			break;
		case '=':
			push_token(TokenTag::GTE, 2);
			break;
		default:
			push_token(TokenTag::GT, 1);
		}
		break;

	case '!':
		switch (next_char()) {
		case '=':
			push_token(TokenTag::NOT_EQUAL, 2);
			break;
		default:
			push_token(TokenTag::LOGIC_COMPL, 1);
		}
		break;

	case '(':
		switch (next_char()) {
		case '<':
			push_token(TokenTag::POLY_OPEN, 2);
			break;
		default:
			push_token(TokenTag::PAREN_OPEN, 1);
		}
		break;

	case ')':
		push_token(TokenTag::PAREN_CLOSE, 1);
		break;
	case '{':
		push_token(TokenTag::BRACE_OPEN, 1);
		break;
	case '}':
		push_token(TokenTag::BRACE_CLOSE, 1);
		break;
	case '[':
		push_token(TokenTag::BRACKET_OPEN, 1);
		break;
	case ']':
		push_token(TokenTag::BRACKET_CLOSE, 1);
		break;
	case '~':
		push_token(TokenTag::COMPL, 1);
		break;
	case ';':
		push_token(TokenTag::SEMICOLON, 1);
		break;
	case '.':
		push_token(TokenTag::DOT, 1);
		break;
	case ',':
		push_token(TokenTag::COMMA, 1);
		break;
	case '@':
		push_token(TokenTag::AT, 1);
		break;
	case '"': {
		m_source_index += 1;
		m_current_column += 1;

		int i0 = m_source_index;
		int c0 = m_current_column;
		int l0 = m_current_line;

		// TODO: support escape sequences
		size_t len = 0;
		while ((not done()) && current_char() != '"') {
			len += 1;
			m_source_index += 1;
			if (current_char() == '\n') {
				m_current_line += 1;
				m_current_column = 0;
			} else {
				m_current_column += 1;
			}
		}

		if (current_char() != '"') {
			// TODO: report unmatched quote
			assert(0);
		}

		char const* base_ptr = m_source.data() + i0;
		m_tokens.push_back(
		    {TokenTag::STRING, InternedString {base_ptr, len}, l0, c0, m_current_line, m_current_column});

		m_current_column += 1;
		m_source_index += 1;
	} break;

	default:
		if (consume_identifier_or_keyword()) {
			return;
		} else if (consume_number()) {
			return;
		} else {
			if (current_char() == '\n') {
				m_current_line += 1;
				m_current_column = 0;
			} else {
				m_current_column += 1;
			}
			m_source_index += 1;
		}
	}
}

bool Lexer::consume_number() {
	if (!isdigit(current_char()))
		return false;

	char const* base_ptr = m_source.data() + m_source_index;
	size_t len = 1;

	bool is_int = true;
	while (isdigit(next_char())) {
		m_source_index += 1;
		m_current_column += 1;
		len += 1;
	}

	if (next_char() == '.') {
		is_int = false;
		m_source_index += 1;
		m_current_column += 1;
		len += 1;

		while (isdigit(next_char())) {
			m_source_index += 1;
			m_current_column += 1;
			len += 1;
		}
	}

	TokenTag type = is_int ? TokenTag::INTEGER : TokenTag::NUMBER;
	m_source_index += 1;
	m_current_column += 1;
	m_tokens.push_back(
	    {type,
	     InternedString {base_ptr, len},
	     m_current_line,
	     m_current_column - int(len),
	     m_current_line,
	     m_current_column});

	return true;
}

bool Lexer::consume_comment() {
	if (current_char() != '/')
		return false;
	m_source_index += 1;
	m_current_column += 1;

	if (current_char() != '/')
		return false;
	m_source_index += 1;
	m_current_column += 1;

	while ((not done()) && current_char() != '\n') {
		m_source_index += 1;
		m_current_column += 1;
	}

	if (current_char() == '\n') {
		m_source_index += 1;
		m_current_line += 1;
		m_current_column = 0;
	}

	return true;
}

void Lexer::advance() {
	m_token_index += 1;
	while (m_token_index >= int(m_tokens.size())) {
		if (done())
			break;
		consume_token();
	}
}

void Lexer::regress() {
	assert(m_token_index > 0);
	m_token_index -= 1;
}

Token const& eof() {
	static Token t = {TokenTag::END, "(EOF)", -1, -1, -1, -1};
	return t;
}

Token const& Lexer::token_at(int index) {
	while (!done() && index >= int(m_tokens.size())) {
		consume_token();
	}

	if (done() && index >= int(m_tokens.size())) {
		return eof();
	}

	return m_tokens.at(index);
}
