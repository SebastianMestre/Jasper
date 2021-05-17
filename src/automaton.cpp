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
	X(Plus, ADD, "+")                                                          \
	X(PlusEq, ADD_TO, "+=")                                                    \
	X(PlusPlus, INCREMENT, "++")                                               \
	X(Minus, SUB, "-")                                                         \
	X(MinusEq, SUB_TO, "-=")                                                   \
	X(MinusMinus, DECREMENT, "--")                                             \
	X(Star, MUL, "*")                                                          \
	X(StarEq, MUL_TO, "*=")                                                    \
	X(Slash, DIV, "/")                                                         \
	X(SlashEq, DIV_TO, "/=")                                                   \
	X(Pipe, IOR, "|")                                                          \
	X(PipeEq, IOR_TO, "|=")                                                    \
	X(PipePipe, LOGIC_IOR, "||")                                               \
	X(Amp, AND, "&")                                                           \
	X(AmpEq, AND_TO, "&=")                                                     \
	X(AmpAmp, LOGIC_AND, "&&")                                                 \
	X(Caret, XOR, "^")                                                         \
	X(CaretEq, XOR_TO, "^=")                                                   \
                                                                               \
	X(Colon, DECLARE, ":")                                                     \
	X(ColonEq, DECLARE_ASSIGN, ":=")                                           \
	X(Assign, ASSIGN, "=")                                                     \
	X(Lt, LT, "<")                                                             \
	X(Lte, LTE, "<=")                                                          \
	X(Eq, EQUAL, "==")                                                         \
	X(Gte, GTE, ">=")                                                          \
	X(Gt, GT, ">")                                                             \
	X(BangEq, NOT_EQUAL, "!=")                                                 \
	X(Bang, LOGIC_COMPL, "!")                                                  \
	X(Tilde, COMPL, "~")                                                       \
	X(Arrow, ARROW, "=>")                                                      \
	X(Pizza, PIZZA, "|>")                                                      \
	X(Semicolon, SEMICOLON, ";")                                               \
	X(Comma, COMMA, ",")                                                       \
	X(Dot, DOT, ".")                                                           \
	X(At, AT, "@")                                                             \
                                                                               \
	X(LParen, PAREN_OPEN, "(")                                                 \
	X(RParen, PAREN_CLOSE, ")")                                                \
	X(LBrace, BRACE_OPEN, "{")                                                 \
	X(RBrace, BRACE_CLOSE, "}")                                                \
	X(LBracket, BRACKET_OPEN, "[")                                             \
	X(RBracket, BRACKET_CLOSE, "]")                                            \
	X(LPoly, POLY_OPEN, "<:")                                                  \
	X(RPoly, POLY_CLOSE, ":>")                                                 \
                                                                               \
	X(Identifier, END, {})                                                     \
	X(String, STRING, {})                                                      \
	X(Comment, END, {})                                                        \
	X(Integer, INTEGER, {})                                                    \
	X(Number, NUMBER, {})

#define X(name, token_tag, string) name,
enum Values { Error, END_STATES Count };
#undef X
#define X(name, token_tag, string) #name,
constexpr char const* end_states[] = { "Error", END_STATES };
#undef X
constexpr int last_fixed_string = RPoly;
}
#define X(name, token_tag, string) string,
static InternedString fixed_strings[] = { END_STATES };
#undef X
#define X(name, token_tag, string) TokenTag::token_tag,
constexpr TokenTag token_tags[] = { END_STATES };
#undef X
#undef END_STATES

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

	// / /= comments
	State saw_slash = new_state();
	State saw_comment_marker = new_state();
	add_transition(result, start, '/', saw_slash);
	add_default_transition(result, saw_slash, Slash);
	add_transition(result, saw_slash, '/', saw_comment_marker);
	add_transition(result, saw_slash, '=', SlashEq);
	add_default_transition(result, saw_comment_marker, saw_comment_marker);
	add_transition(result, saw_comment_marker, '\n', Comment);
	add_transition(result, saw_comment_marker, '\0', Comment);

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

	// = == =>
	State saw_eq = new_state();
	add_transition(result, start, '=', saw_eq);
	add_default_transition(result, saw_eq, Assign);
	add_transition(result, saw_eq, '=', Eq);
	add_transition(result, saw_eq, '>', Arrow);

	// ! !=
	State saw_bang = new_state();
	add_transition(result, start, '!', saw_bang);
	add_default_transition(result, saw_bang, Bang);
	add_transition(result, saw_bang, '=', BangEq);

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

	// > >=
	State saw_gt = new_state();
	add_transition(result, start, '>', saw_gt);
	add_default_transition(result, saw_gt, Gt);
	add_transition(result, saw_gt, '=', Gte);

	// + += ++
	State saw_plus = new_state();
	add_transition(result, start, '+', saw_plus);
	add_default_transition(result, saw_plus, Plus);
	add_transition(result, saw_plus, '=', PlusEq);
	add_transition(result, saw_plus, '+', PlusPlus);

	// - -= --
	State saw_dash = new_state();
	add_transition(result, start, '-', saw_dash);
	add_default_transition(result, saw_dash, Minus);
	add_transition(result, saw_dash, '=', MinusEq);
	add_transition(result, saw_dash, '-', MinusMinus);

	// * *=
	State saw_star = new_state();
	add_transition(result, start, '*', saw_star);
	add_default_transition(result, saw_star, Star);
	add_transition(result, saw_star, '=', StarEq);

	// | |> || |=
	State saw_pipe = new_state();
	add_transition(result, start, '|', saw_pipe);
	add_default_transition(result, saw_pipe, Pipe);
	add_transition(result, saw_pipe, '=', PipeEq);
	add_transition(result, saw_pipe, '|', PipePipe);
	add_transition(result, saw_pipe, '>', Pizza);

	// & && &=
	State saw_amp = new_state();
	add_transition(result, start, '&', saw_amp);
	add_default_transition(result, saw_amp, Amp);
	add_transition(result, saw_amp, '=', AmpEq);
	add_transition(result, saw_amp, '&', AmpAmp);

	// ^ ^=
	State saw_caret = new_state();
	add_transition(result, start, '^', saw_caret);
	add_default_transition(result, saw_caret, Caret);
	add_transition(result, saw_caret, '=', CaretEq);

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
	add_transition(result, start, '~', Tilde);
	add_transition(result, start, '@', At);

#undef new_state
}

constexpr void init_offsets(Automaton& result) {
	using namespace EndStates;

	State states_that_need_to_step_back_after_completing[] = {
		Identifier,
		Integer,
		Number,
		Colon,
		Assign,
		Pipe,
		Amp,
		Bang,
		Caret,
		Plus,
		Minus,
		Star,
		Slash,
		Lt,
		Gt,
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

}

namespace KeywordLexer {

namespace EndStates {

#define END_STATES                                                             \
	X(If, KEYWORD_IF, "if")                                                    \
	X(For, KEYWORD_FOR, "for")                                                 \
	X(Else, KEYWORD_ELSE, "else")                                              \
	X(Fn, KEYWORD_FN, "fn")                                                    \
	X(Then, KEYWORD_THEN, "then")                                              \
	X(Return, KEYWORD_RETURN, "return")                                        \
	X(While, KEYWORD_WHILE, "while")                                           \
	X(Match, KEYWORD_MATCH, "match")                                           \
	X(True, KEYWORD_TRUE, "true")                                              \
	X(False, KEYWORD_FALSE, "false")                                           \
	X(Array, KEYWORD_ARRAY, "array")                                           \
	X(Null, KEYWORD_NULL, "null")                                              \
	X(Seq, KEYWORD_SEQ, "seq")                                                 \
	X(Tuple, KEYWORD_TUPLE, "tuple")                                           \
	X(Struct, KEYWORD_STRUCT, "struct")                                        \
	X(Union, KEYWORD_UNION, "union")

#define X(name, token_tag, string) name,
enum Values { Error, END_STATES Count };
#undef X
#define X(name, token_tag, string) #name,
constexpr char const* end_states[] = { "Error", END_STATES };
#undef X
}
#define X(name, token_tag, string) TokenTag::token_tag,
constexpr TokenTag token_tags[] = { END_STATES };
#undef X
#define X(name, token_tag, string) string,
static InternedString fixed_strings[] = { END_STATES };
#undef X
#undef END_STATES

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
			0, 0, 0, 0
		});
	} else {
		ta.push_back({
			KeywordLexer::token_tags[state - 1],
			KeywordLexer::fixed_strings[state - 1],
			0, 0, 0, 0,
		});
	}
}

TokenArray tokenize(char const* p) {
	// Implementation detail: We store indices into the source buffer
	// in the source_locations at first, then resolve them in another pass.

	char const* const code_start = p;

	constexpr Automaton a = MainLexer::make();
	constexpr Automaton ka = KeywordLexer::make();

	TokenArray ta;
	while (*p == ' ' || *p == '\t' || *p == '\n') ++p;
	while (*p != '\0') {
		char const* const p0 = p;

		int state = state_count - 1;
		while (MainLexer::EndStates::Count < state) {
			state = a.go(state, *p++);
		}

		if (state == MainLexer::EndStates::Error) {
			print_error(p);
			break;
		}

		p += a.offset[state];

		if(state <= MainLexer::EndStates::last_fixed_string) {
			ta.push_back({
				MainLexer::token_tags[state - 1],
				MainLexer::fixed_strings[state - 1],
				{{p0 - code_start}, {p - code_start}}
			});
		} else if(state == MainLexer::EndStates::Identifier) {
			push_identifier_or_keyword(ka, ta, string_view(p0, p - p0));
		} else if(state == MainLexer::EndStates::Comment) {
			// do nothing.
		} else if(state == MainLexer::EndStates::String) {
			ta.push_back({
				MainLexer::token_tags[state - 1],
				InternedString(p0 + 1, p - p0 - 2),
				{{p0 - code_start}, {p - code_start}}
			});
		} else {
			ta.push_back({
				MainLexer::token_tags[state - 1],
				InternedString(p0, p - p0),
				{{p0 - code_start}, {p - code_start}}
			});
		}

		while (*p == ' ' || *p == '\t' || *p == '\n') ++p;
	}
	ta.push_back({TokenTag::END, InternedString()});

	// resolve source locations
	{
		int const real_token_count = ta.size() - 1;
		int code_idx = 0;
		int line = 0;
		int col = 0;
		for (int i = 0; i < real_token_count; ++i) {
			auto& token = ta.at(i);

			int code_idx1 = token.m_source_location.start.line;
			int code_idx2 = token.m_source_location.end.line;

			for (; code_idx < code_idx1; ++code_idx) {
				if (code_start[code_idx] == '\n') {
					line += 1;
					col = 0;
				} else {
					col += 1;
				}
			}

			token.m_source_location.start.line = line;
			token.m_source_location.start.col = col;

			for (; code_idx < code_idx2; ++code_idx) {
				if (code_start[code_idx] == '\n') {
					line += 1;
					col = 0;
				} else {
					col += 1;
				}
			}

			token.m_source_location.end.line = line;
			token.m_source_location.end.col = col;
		}
	}

	return ta;
}
