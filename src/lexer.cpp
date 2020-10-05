#include "lexer.hpp"

#include <string>
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

bool Lexer::consume_keyword() {
	// TODO: make table based?
	char c0 = peek_char(0);
	switch (c0) {
	case 'a': {
		if (peek_char(1) == 'r' && peek_char(2) == 'r' && peek_char(3) == 'a' &&
		    peek_char(4) == 'y' && not is_identifier_char(peek_char(5))) {
			push_token(TokenTag::KEYWORD_ARRAY, 5);
			return true;
		}
	} break;
	case 'f': {
		char c1 = peek_char(1);
		if (c1 == 'n') {
			if (not is_identifier_char(peek_char(2))) {
				push_token(TokenTag::KEYWORD_FN, 2);
				return true;
			}
		} else if (c1 == 'o') {
			char c2 = peek_char(2);
			if (c2 == 'r') {
				if (not is_identifier_char(peek_char(3))) {
					push_token(TokenTag::KEYWORD_FOR, 3);
					return true;
				}
			}
		} else if (
		    peek_char(1) == 'a' && peek_char(2) == 'l' && peek_char(3) == 's' &&
		    peek_char(4) == 'e' && not is_identifier_char(peek_char(5))) {
			push_token(TokenTag::KEYWORD_FALSE, 5);
			return true;
		}
	} break;
	case 'i': {
		char c1 = peek_char(1);
		if (c1 == 'f') {
			if (not is_identifier_char(peek_char(2))) {
				push_token(TokenTag::KEYWORD_IF, 2);
				return true;
			}
		}
	} break;
	case 'r':
		if (peek_char(1) == 'e' && peek_char(2) == 't' && peek_char(3) == 'u' &&
		    peek_char(4) == 'r' && peek_char(5) == 'n' &&
		    not is_identifier_char(peek_char(6))) {
			push_token(TokenTag::KEYWORD_RETURN, 6);
			return true;
		}
		break;
	case 'd':
		if (peek_char(1) == 'i' && peek_char(2) == 'c' && peek_char(3) == 't' &&
		    not is_identifier_char(peek_char(4))) {
			push_token(TokenTag::KEYWORD_DICT, 4);
			return true;
		}
		break;
	case 'o':
		if (peek_char(1) == 'b' && peek_char(2) == 't' &&
		    not is_identifier_char(peek_char(3))) {
			push_token(TokenTag::KEYWORD_OBJECT, 3);
			return true;
		}
		break;
	case 't':
		if (peek_char(1) == 'r' && peek_char(2) == 'u' && peek_char(3) == 'e' &&
		    not is_identifier_char(peek_char(4))) {
			push_token(TokenTag::KEYWORD_TRUE, 4);
			return true;
		} else if (
		    peek_char(1) == 'h' && peek_char(2) == 'e' && peek_char(3) == 'n' &&
		    not is_identifier_char(peek_char(4))) {
			push_token(TokenTag::KEYWORD_THEN, 4);
			return true;
		} else if (
		    peek_char(1) == 'u' && peek_char(2) == 'p' && peek_char(3) == 'l' &&
		    peek_char(4) == 'e' && not is_identifier_char(peek_char(5))) {
			push_token(TokenTag::KEYWORD_TUPLE, 5);
			return true;
		}
		break;
	case 'n':
		if (peek_char(1) == 'u' && peek_char(2) == 'l' && peek_char(3) == 'l' &&
		    not is_identifier_char(peek_char(4))) {
			push_token(TokenTag::KEYWORD_NULL, 4);
			return true;
		}
		break;
	case 'w':
		if (peek_char(1) == 'h' && peek_char(2) == 'i' && peek_char(3) == 'l' &&
		    peek_char(4) == 'e' && not is_identifier_char(peek_char(5))) {
			push_token(TokenTag::KEYWORD_WHILE, 5);
			return true;
		}
		break;
	case 'e':
		if (peek_char(1) == 'l' && peek_char(2) == 's' && peek_char(3) == 'e' &&
		    not is_identifier_char(peek_char(4))) {
			push_token(TokenTag::KEYWORD_ELSE, 4);
			return true;
		}
		break;
	case 'u':
		if (peek_char(1) == 'n' && peek_char(2) == 'i' && peek_char(3) == 'o' &&
		    peek_char(4) == 'n' && not is_identifier_char(peek_char(5))) {
			push_token(TokenTag::KEYWORD_UNION, 5);
			return true;
		}
		break;
	case 's':
		if (peek_char(1) == 't' && peek_char(2) == 'r' && peek_char(3) == 'u' &&
		    peek_char(4) == 'c' && peek_char(5) == 't' &&
		    not is_identifier_char(peek_char(6))) {
			push_token(TokenTag::KEYWORD_STRUCT, 6);
			return true;
		}
		break;
	}
	return false;
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
		int len = 0;
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

		std::string text;
		assert(len == (m_source_index - i0));
		text.reserve(len);
		for (int i = i0; i != m_source_index; ++i) {
			text.push_back(m_source[i]);
		}

		m_tokens.push_back(
		    {TokenTag::STRING, InternedString {text}, l0, c0, m_current_line, m_current_column});

		m_current_column += 1;
		m_source_index += 1;
	} break;

	default:
		if (consume_keyword())
			return;

		if (is_identifier_start_char(current_char())) {
			std::string text;
			text.push_back(current_char());

			while (is_identifier_char(next_char())) {
				m_source_index += 1;
				text.push_back(current_char());
			}

			m_source_index += 1;
			m_current_column += text.size();

			m_tokens.push_back(
			    {TokenTag::IDENTIFIER,
			     InternedString {text},
			     m_current_line,
			     m_current_column - int(text.size()),
			     m_current_line,
			     m_current_column});

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

	std::string text;
	text.push_back(current_char());

	bool is_int = true;
	while (isdigit(next_char())) {
		m_source_index += 1;
		m_current_column += 1;
		text.push_back(current_char());
	}

	if (next_char() == '.') {
		is_int = false;
		m_source_index += 1;
		m_current_column += 1;
		text.push_back(current_char());

		while (isdigit(next_char())) {
			m_source_index += 1;
			m_current_column += 1;
			text.push_back(current_char());
		}
	}

	TokenTag type = is_int ? TokenTag::INTEGER : TokenTag::NUMBER;
	m_source_index += 1;
	m_current_column += 1;
	m_tokens.push_back(
	    {type,
	     InternedString {text},
	     m_current_line,
	     m_current_column - int(text.size()),
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
