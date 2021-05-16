#include "automaton.hpp"

#include "token.hpp"
#include "token_array.hpp"
#include "utils/string_view.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>

static constexpr int state_count = 200;
using State = uint8_t;

struct Automaton {
	int offset[state_count];
	State transition[state_count][256] {};

	constexpr State go(State s, unsigned char c) const {
		return transition[s][c];
	}
};

constexpr void add_default_transition(Automaton& a, State src, State dst) {
	for (int i = 256; i--;)
		a.transition[src][i] = dst;
}

constexpr void add_transition(Automaton& a, State src, unsigned char c, State dst) {
	a.transition[src][c] = dst;
}

constexpr void add_string(Automaton& a, int& state_counter, State end_state, char const* str) {
#define new_state() (--state_counter)
	int state = state_count - 1;
	while (*str) {
		if (*(str+1) == '\0')
			add_transition(a, state, *str, end_state);
		else if (a.go(state, *str) == 0)
			add_transition(a, state, *str, new_state());
		state = a.go(state, *str++);
	}
#undef new_state
}

namespace MainLexer {

namespace EndStates {

#define END_STATES                                                             \
	X(Error)                                                                   \
	X(Semicolon)                                                               \
	X(Comma)                                                                   \
	X(Dot)                                                                     \
	X(LParen)                                                                  \
	X(RParen)                                                                  \
	X(LBrace)                                                                  \
	X(RBrace)                                                                  \
	X(LBracket)                                                                \
	X(RBracket)                                                                \
	X(Colon)                                                                   \
	X(ColonEq)                                                                 \
	X(LPoly)                                                                   \
	X(RPoly)                                                                   \
	X(Lt)                                                                      \
	X(Lte)                                                                     \
	X(Eq)                                                                      \
	X(Plus)                                                                    \
	X(PlusEq)                                                                  \
	X(Minus)                                                                   \
	X(MinusEq)                                                                 \
	X(Pipe)                                                                    \
	X(Pizza)                                                                   \
	X(Assign)                                                                  \
	X(Arrow)                                                                   \
	X(Identifier)                                                              \
	X(String)                                                                  \
	X(Comment)                                                                 \
	X(Integer)                                                                 \
	X(Number)

#define X(name) name,
enum Values { END_STATES Count };
#undef X
#define X(name) #name,
constexpr char const* end_states[] = { END_STATES };
#undef X
constexpr int last_fixed_string = Arrow;

#undef END_STATES
}

constexpr void init_transitions(Automaton& result) {
	// TODO: CLRF, more error handling

	using namespace EndStates;
	int normal_states = state_count;

#define new_state() (--normal_states)

	for (int i = state_count; i--;)
		add_default_transition(result, i, Error);

	State start = new_state(); // state_count - 1

	// Identifiers
	State saw_id_char = new_state();
	add_transition(result, start, '_', saw_id_char);
	for (char c = 'a'; c <= 'z'; ++c) add_transition(result, start, c, saw_id_char);
	for (char c = 'A'; c <= 'Z'; ++c) add_transition(result, start, c, saw_id_char);
	add_default_transition(result, saw_id_char, Identifier);
	add_transition(result, saw_id_char, '_', saw_id_char);
	for (char c = 'a'; c <= 'z'; ++c) add_transition(result, saw_id_char, c, saw_id_char);
	for (char c = 'A'; c <= 'Z'; ++c) add_transition(result, saw_id_char, c, saw_id_char);
	for (char c = '0'; c <= '9'; ++c) add_transition(result, saw_id_char, c, saw_id_char);

	// String literals
	State saw_open_string = new_state();
	add_transition(result, start, '"', saw_open_string);
	add_default_transition(result, saw_open_string, saw_open_string);
	add_transition(result, saw_open_string, '"', String);

	// comments
	State saw_slash = new_state();
	State saw_comment_marker = new_state();
	add_transition(result, start, '/', saw_slash);
	add_transition(result, saw_slash, '/', saw_comment_marker);
	add_default_transition(result, saw_comment_marker, saw_comment_marker);
	add_transition(result, saw_comment_marker, '\n', Comment);
	add_transition(result, saw_comment_marker, EOF, Comment);

	// numbers
	State saw_digit = new_state();
	State saw_number_dot = new_state();
	State saw_decimal_digit = new_state();
	for (char c = '0'; c <= '9'; ++c) add_transition(result, start, c, saw_digit);
	add_default_transition(result, saw_digit, Integer);
	for (char c = '0'; c <= '9'; ++c) add_transition(result, saw_digit, c, saw_digit);
	add_transition(result, saw_digit, '.', saw_number_dot);
	for (char c = '0'; c <= '9'; ++c) add_transition(result, saw_number_dot, c, saw_decimal_digit);
	add_default_transition(result, saw_decimal_digit, Number);
	for (char c = '0'; c <= '9'; ++c) add_transition(result, saw_decimal_digit, c, saw_decimal_digit);

	// = ==
	State saw_eq = new_state();
	add_transition(result, start, '=', saw_eq);
	add_default_transition(result, saw_eq, Assign);
	add_transition(result, saw_eq, '=', Eq);
	add_transition(result, saw_eq, '>', Arrow);

	// : := :>
	State saw_colon = new_state();
	add_transition(result, start, ':', saw_colon);
	add_default_transition(result, saw_colon, Colon);
	add_transition(result, saw_colon, '=', ColonEq);
	add_transition(result, saw_colon, '>', RPoly);

	// < <= <:
	State saw_lt = new_state();
	add_transition(result, start, '<', saw_lt);
	add_default_transition(result, saw_lt, Lt);
	add_transition(result, saw_lt, '=', Lte);
	add_transition(result, saw_lt, ':', LPoly);

	// + +=
	State saw_plus = new_state();
	add_transition(result, start, '+', saw_plus);
	add_default_transition(result, saw_plus, Plus);
	add_transition(result, saw_plus, '=', PlusEq);

	// - -=
	State saw_dash = new_state();
	add_transition(result, start, '-', saw_dash);
	add_default_transition(result, saw_dash, Minus);
	add_transition(result, saw_dash, '=', MinusEq);


	// | |>
	State saw_pipe = new_state();
	add_transition(result, start, '|', saw_pipe);
	add_default_transition(result, saw_pipe, Pipe);
	add_transition(result, saw_pipe, '>', Pizza);

	// single char symbols
	add_transition(result, start, ';', Semicolon);
	add_transition(result, start, ',', Comma);
	add_transition(result, start, '.', Dot);
	add_transition(result, start, '(', LParen);
	add_transition(result, start, ')', RParen);
	add_transition(result, start, '{', LBrace);
	add_transition(result, start, '}', RBrace);
	add_transition(result, start, '[', LBracket);
	add_transition(result, start, ']', RBracket);

#undef new_state
}

constexpr void init_offsets(Automaton& result) {
	using namespace EndStates;

	State states_that_need_to_step_back_after_completing[] = {
		Identifier,
		Integer,
		Number,
		Colon,
		Lt,
		Assign,
		Plus,
		Assign,
		Pipe,
		Minus,
	};

	for (State state : states_that_need_to_step_back_after_completing)
		result.offset[state] = -1;
}

constexpr Automaton make() {
	Automaton result{};
	init_transitions(result);
	init_offsets(result);
	return result;
}

static std::pair<InternedString, TokenTag> symbols[] = {
	{";", TokenTag::SEMICOLON },
	{",", TokenTag::COMMA },
	{".", TokenTag::DOT },
	{"(", TokenTag::PAREN_OPEN },
	{")", TokenTag::PAREN_CLOSE },
	{"{", TokenTag::BRACE_OPEN },
	{"}", TokenTag::BRACE_CLOSE },
	{"[", TokenTag::BRACKET_OPEN },
	{"]", TokenTag::BRACKET_CLOSE },
	{":", TokenTag::DECLARE },
	{":=", TokenTag::DECLARE_ASSIGN },
	{"<:", TokenTag::POLY_OPEN },
	{":>", TokenTag::POLY_CLOSE },
	{"<", TokenTag::LT },
	{"<=", TokenTag::LTE },
	{"==", TokenTag::EQUAL },
	{"+", TokenTag::ADD },
	{"+=", TokenTag::ADD_TO },
	{"-", TokenTag::SUB },
	{"-=", TokenTag::SUB_TO },
	{"|", TokenTag::IOR },
	{"|>", TokenTag::PIZZA },
	{"=", TokenTag::ASSIGN },
	{"=>", TokenTag::ARROW },
};

}

namespace KeywordLexer {

namespace EndStates {

#define END_STATES                                                             \
	X(Error)                                                                   \
	X(If)                                                                      \
	X(For)                                                                     \
	X(Else)                                                                    \
	X(Fn)                                                                      \
	X(Then)                                                                    \
	X(Return)                                                                  \
	X(While)                                                                   \
	X(Match)                                                                   \
	X(True)                                                                    \
	X(False)                                                                   \
	X(Array)                                                                   \
	X(Null)                                                                    \
	X(Seq)                                                                     \
	X(Tuple)                                                                   \
	X(Struct)                                                                  \
	X(Union)

#define X(name) name,
enum Values { END_STATES Count };
#undef X
#define X(name) #name,
constexpr char const* end_states[] = { END_STATES };
#undef X

#undef END_STATES

}

static std::pair<InternedString, TokenTag> keywords[] = {
	{"if", TokenTag::KEYWORD_IF },
	{"for", TokenTag::KEYWORD_FOR},
	{"else", TokenTag::KEYWORD_ELSE},
	{"fn", TokenTag::KEYWORD_FN},
	{"then", TokenTag::KEYWORD_THEN},
	{"return", TokenTag::KEYWORD_RETURN},
	{"while", TokenTag::KEYWORD_WHILE},
	{"match", TokenTag::KEYWORD_MATCH},
	{"true", TokenTag::KEYWORD_TRUE},
	{"false", TokenTag::KEYWORD_FALSE},
	{"array", TokenTag::KEYWORD_ARRAY},
	{"null", TokenTag::KEYWORD_NULL},
	{"seq", TokenTag::KEYWORD_SEQ},
	{"tuple", TokenTag::KEYWORD_TUPLE},
	{"struct", TokenTag::KEYWORD_STRUCT},
	{"union", TokenTag::KEYWORD_UNION},
};


constexpr Automaton make() {
#define new_state() (--state_counter)
	Automaton a{};

	int state_counter = state_count;

	int start_state = new_state();

	add_string(a, state_counter, EndStates::If, "if");
	add_string(a, state_counter, EndStates::For, "for");
	add_string(a, state_counter, EndStates::Else, "else");
	add_string(a, state_counter, EndStates::Fn, "fn");
	add_string(a, state_counter, EndStates::Then, "then");
	add_string(a, state_counter, EndStates::Return, "return");
	add_string(a, state_counter, EndStates::While, "while");
	add_string(a, state_counter, EndStates::Match, "match");
	add_string(a, state_counter, EndStates::True, "true");
	add_string(a, state_counter, EndStates::False, "false");
	add_string(a, state_counter, EndStates::Array, "array");
	add_string(a, state_counter, EndStates::Null, "null");
	add_string(a, state_counter, EndStates::Seq, "seq");
	add_string(a, state_counter, EndStates::Tuple, "tuple");
	add_string(a, state_counter, EndStates::Struct, "struct");
	add_string(a, state_counter, EndStates::Union, "union");

	return a;
#undef new_state
}

}

static void print_error(char const* p) {
	printf("Error -- last two chars are: %c%c\n", *(p-2), *(p-1));
}

static void push_identifier_or_keyword(Automaton const& a, TokenArray& ta, string_view str) {

	int state = state_count - 1;
	for (int i = 0; i < str.size(); ++i) {
		state = a.go(state, str.begin()[i]);
		if (state == KeywordLexer::EndStates::Error)
			break;
	}

	if (KeywordLexer::EndStates::Count <= state || state == KeywordLexer::EndStates::Error) {
		ta.push_back({
			TokenTag::IDENTIFIER,
			InternedString(str.begin(), str.size()),
			0,0,0,0
		});
	} else {
		auto& pair = KeywordLexer::keywords[state - 1];
		ta.push_back({
			pair.second,
			pair.first,
			0,0,0,0,
		});
	}
}

TokenArray tokenize(char const* p) {
	constexpr Automaton a = MainLexer::make();
	constexpr Automaton ka = KeywordLexer::make();

	TokenArray ta;
	while (*p == ' ' || *p == '\t' || *p == '\n') ++p;
	while (*p != EOF) {
		int state = state_count - 1;
		char const* p0 = p;
		while (MainLexer::EndStates::Count < state) {
			state = a.go(state, *p++);
		}

		if (state == MainLexer::EndStates::Error) {
			print_error(p);
			break;
		}

		p += a.offset[state];

		if(state < MainLexer::EndStates::last_fixed_string) {
			auto const& pair = MainLexer::symbols[state - 1];
			ta.push_back({
				pair.second,
				pair.first,
				0,0,0,0
			});
		} else if(state == MainLexer::EndStates::Identifier) {
			push_identifier_or_keyword(ka, ta, string_view(p0, p-p0));
		} else {
			// TODO: fix tag
			ta.push_back({
				static_cast<TokenTag>(state),
				InternedString(p0, p-p0),
				0,0,0,0
			});
		}

		while (*p == ' ' || *p == '\t' || *p == '\n') ++p;
	}
	return ta;
}
