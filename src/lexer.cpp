#include "lexer.hpp"

#include <vector>

#include <cassert>
#include <cctype>
#include <cstdlib>

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

	std::vector<char> m_source;
	TokenArray& m_tokens;

	Trie m_symbols_trie;

	bool m_done {false};

	int m_source_index {0};
	int m_token_index {0};

	int m_current_line {0};
	int m_current_column {0};
};

TokenArray tokenize(std::string const& source) {
	TokenArray ta;
	std::vector<char> v;
	v.reserve(source.size());
	for (char c : source)
		v.push_back(c);
	Lexer lexer = {std::move(v), ta};
	while (not lexer.done())
		lexer.consume_token();
	return ta;
}

static Token const& eof() {
	static Token t = {TokenTag::END, "(EOF)", -1, -1, -1, -1};
	return t;
}

Lexer::Lexer(std::vector<char> source, TokenArray& tokens)
    : m_source {std::move(source)}
    , m_tokens {tokens}
    , m_symbols_trie {build_trie({{"+", int(TokenTag::ADD)},
                                  {"+=", int(TokenTag::ADD_TO)},
                                  {"-", int(TokenTag::SUB)},
                                  {"-=", int(TokenTag::SUB_TO)},
                                  {"*", int(TokenTag::MUL)},
                                  {"*=", int(TokenTag::MUL_TO)},
                                  {"/", int(TokenTag::DIV)},
                                  {"/=", int(TokenTag::DIV_TO)},
                                  {"&", int(TokenTag::AND)},
                                  {"&=", int(TokenTag::AND_TO)},
                                  {"|", int(TokenTag::IOR)},
                                  {"|=", int(TokenTag::IOR_TO)},
                                  {"^", int(TokenTag::XOR)},
                                  {"^=", int(TokenTag::XOR_TO)},
                                  {"++", int(TokenTag::INCREMENT)},
                                  {"--", int(TokenTag::DECREMENT)},
                                  {"&&", int(TokenTag::LOGIC_AND)},
                                  {"||", int(TokenTag::LOGIC_IOR)},
                                  {"~", int(TokenTag::COMPL)},
                                  {"!", int(TokenTag::LOGIC_COMPL)},
                                  {":", int(TokenTag::DECLARE)},
                                  {":=", int(TokenTag::DECLARE_ASSIGN)},
                                  {"=", int(TokenTag::ASSIGN)},
                                  {"=>", int(TokenTag::ARROW)},
                                  {"==", int(TokenTag::EQUAL)},
                                  {"!=", int(TokenTag::NOT_EQUAL)},
                                  {"<", int(TokenTag::LT)},
                                  {"<=", int(TokenTag::LTE)},
                                  {">", int(TokenTag::GT)},
                                  {">=", int(TokenTag::GTE)},
                                  {"<:", int(TokenTag::POLY_OPEN)},
                                  {":>", int(TokenTag::POLY_CLOSE)},
                                  {"(", int(TokenTag::PAREN_OPEN)},
                                  {")", int(TokenTag::PAREN_CLOSE)},
                                  {"{", int(TokenTag::BRACE_OPEN)},
                                  {"}", int(TokenTag::BRACE_CLOSE)},
                                  {"[", int(TokenTag::BRACKET_OPEN)},
                                  {"]", int(TokenTag::BRACKET_CLOSE)},
                                  {";", int(TokenTag::SEMICOLON)},
                                  {",", int(TokenTag::COMMA)},
                                  {".", int(TokenTag::DOT)},
                                  {"|>", int(TokenTag::PIZZA)}})} {}

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
		{{"null"}, TokenTag::KEYWORD_NULL},
		{{"seq"}, TokenTag::KEYWORD_SEQ},
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

bool Lexer::consume_string() {
	if (current_char() != '"')
		return false;

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
	m_tokens.push_back({
		TokenTag::STRING,
		InternedString {base_ptr, len},
		l0, c0,
		m_current_line, m_current_column});

	m_current_column += 1;
	m_source_index += 1;

	return true;
}

bool Lexer::consume_symbol() {
	auto entry = m_symbols_trie.longest_prefix_of(string_view {
	    m_source.data() + m_source_index, m_source.size() - m_source_index});
	if (entry.text.begin() == nullptr)
		return false;
	push_token(TokenTag(entry.data), entry.text.size());
	return true;
}

void Lexer::consume_token() {
	if (done()) return;

	if (current_char() == '\0') {
		m_tokens.push_back(eof());
		m_done = true;
		return;
	}

	if (current_char() == '/' && next_char() == '/') {
		if (!consume_comment())
			assert(0);
		return;
	}

	if (consume_symbol()) {
		return;
	}

	if (current_char() == '"') {
		if (!consume_string())
			assert(0);
		return;
	}

	if (consume_identifier_or_keyword())
		return;

	if (consume_number())
		return;

	if (current_char() == '\n') {
		m_current_line += 1;
		m_current_column = 0;
	} else {
		m_current_column += 1;
	}

	m_source_index += 1;
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
