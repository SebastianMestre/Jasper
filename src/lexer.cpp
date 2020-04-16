#include "lexer.hpp"

#include <iostream>
#include <string>

#include <cstdlib>

char Lexer::char_at(int index) {
	return index < int(m_source.size())
		? m_source[index]
		: '\0';
}

void Lexer::push_token (token_type tt, int width) {
	Token t = {
		tt,
		{},
		m_current_line, m_current_column,
		m_current_line, m_current_column+width
	};

	m_tokens.push_back(t);

	m_source_index += width;
	m_current_column += width;
}

void Lexer::consume_token () {
	switch(current_char()){

		case '+':
			switch(next_char()){
				case '+': push_token(token_type::INCREMENT, 2); break;
				case '=': push_token(token_type::ADD_TO, 2); break;
				default: push_token(token_type::ADD, 1);
			}
			break;

		case '-':
			switch(next_char()){
				case '-': push_token(token_type::DECREMENT, 2); break;
				case '=': push_token(token_type::SUB_TO, 2); break;
				default: push_token(token_type::SUB, 1);
			}
			break;

		case '*':
			switch(next_char()){
				case '=': push_token(token_type::MUL_TO, 2); break;
				default: push_token(token_type::MUL, 1);
			}
			break;

		case '/':
			switch(next_char()){
				case '=': push_token(token_type::DIV_TO, 2); break;
				default: push_token(token_type::DIV, 1);
			}
			break;

		case '&':
			switch(next_char()){
				case '&': push_token(token_type::LOGIC_AND, 2); break;
				case '=': push_token(token_type::AND_TO, 2); break;
				default: push_token(token_type::AND, 1);
			}
			break;

		case '|':
			switch(next_char()){
				case '|': push_token(token_type::LOGIC_IOR, 2); break;
				case '=': push_token(token_type::IOR_TO, 2); break;
				default: push_token(token_type::IOR, 1);
			}
			break;

		case '^':
			switch(next_char()){
				/* case '^': push_token(token_type::LOGIC_XOR, 2); break; */
				case '=': push_token(token_type::XOR_TO, 2); break;
				default: push_token(token_type::XOR, 1);
			}
			break;

		case ':':
			switch(next_char()){
				case '=': push_token(token_type::DECLARE_ASSIGN, 2); break;
				default: push_token(token_type::DECLARE, 1);
			}
			break;

		case '=':
			switch(next_char()){
				case '=': push_token(token_type::EQUAL, 2); break;
				default: push_token(token_type::ASSIGN, 1);
			}
			break;

		case '!':
			switch(next_char()){
				case '=': push_token(token_type::NOT_EQUAL, 2); break;
				default: push_token(token_type::LOGIC_COMPL, 1);
			}
			break;

		case '(': push_token(token_type::PAREN_OPEN, 1); break;
		case ')': push_token(token_type::PAREN_CLOSE, 1); break;
		case '{': push_token(token_type::BRACE_OPEN, 1); break;
		case '}': push_token(token_type::BRACE_CLOSE, 1); break;
		case '[': push_token(token_type::BRACKET_OPEN, 1); break;
		case ']': push_token(token_type::BRACKET_CLOSE, 1); break;
		case '~': push_token(token_type::COMPL, 1); break;
		case ';': push_token(token_type::SEMICOLON, 1); break;

		default:
			if(isalpha(current_char())){
				std::string text;

				/* consume letters */
				do {
					text.push_back(current_char());
					m_source_index++;
				} while(isalpha(current_char()));

				/* adjust column pointer */
				m_curent_column += text.size();

				m_tokens.push_back({
						token_type::IDENTIFIER,
						text
						});
			} else if(isdigit(current_char())) {
				std::string text;

				/* consume numbers */
				do {
					text.push_back(current_char());
					m_source_index++;
				} while(isdigit(current_char()));

				/* it's a float */
				if(next_char() == '.'){
					text.push_back(current_char());
					m_source_index++;
				}

				/* consume more numbers, if any. If not, it's wrong */
				while(isdigit(current_char())) {
					text.push_back(current_char());
					m_source_index++;
				}

				/* adjust column pointer */
				m_current_column += text.size();

				m_tokens.push_back({
						token_type::NUMBER,
						text
						});

			} else {
				if(current_char() == '\n') {
					m_current_line += 1;
					m_current_column = 0;
				}else{
					m_current_column += 1;
				}
				m_source_index += 1;
			}
	}
}

void Lexer::advance () {
	m_token_index += 1;
	while(m_token_index >= int(m_tokens.size())){
		if(done()) break;
		consume_token();
	}

}

void Lexer::regress () {
	assert(m_token_index > 0);
	m_token_index -= 1;
}

Token const& Lexer::token_at (int index) {
	while(!done() && index >= int(m_tokens.size())){
		consume_token();
	}

	if(done() && index >= int(m_tokens.size())) {
		std::cerr << "requested for token after EOF ("<<index<<")\n";
		exit(EXIT_FAILURE);
	}

	return m_tokens.at(index);
}

