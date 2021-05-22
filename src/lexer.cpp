#include "automaton.hpp"

#include "token.hpp"
#include "token_array.hpp"
#include "utils/string_view.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>

constexpr void add_string(AutomatonBuilder& builder, char const* str, State end_state) {
	int state = state_count - 1;
	while (*str) {
		if (*(str+1) == '\0')
			builder.transition(state, *str, end_state);
		else if (builder.automaton.go(state, *str) == 0)
			builder.transition(state, *str, builder.new_state());
		state = builder.automaton.go(state, *str++);
	}
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

} // namespace EndStates

#define X(name, token_tag, string) string,
static InternedString fixed_strings[] = { END_STATES };
#undef X

#define X(name, token_tag, string) TokenTag::token_tag,
constexpr TokenTag token_tags[] = { END_STATES };
#undef X

#undef END_STATES

constexpr void init_transitions(AutomatonBuilder& builder) {
	// TODO: CLRF, more error handling

	using namespace EndStates;

	for (int i = state_count; i--;)
		builder.default_transition(i, Error);

	State start = builder.new_state();
	State saw_id_char = builder.new_state();
	State saw_open_string = builder.new_state();
	State saw_slash = builder.new_state();
	State saw_comment_marker = builder.new_state();
	State saw_digit = builder.new_state();
	State saw_number_dot = builder.new_state();
	State saw_decimal_digit = builder.new_state();
	State saw_eq = builder.new_state();
	State saw_bang = builder.new_state();
	State saw_colon = builder.new_state();
	State saw_lt = builder.new_state();
	State saw_gt = builder.new_state();
	State saw_plus = builder.new_state();
	State saw_dash = builder.new_state();
	State saw_star = builder.new_state();
	State saw_pipe = builder.new_state();
	State saw_amp = builder.new_state();
	State saw_caret = builder.new_state();

	builder

	.from_state(start)
		.transition('_', saw_id_char)
		.transition(Range {'a', 'z'}, saw_id_char)
		.transition(Range {'A', 'Z'}, saw_id_char)
		.transition('"', saw_open_string)
		.transition(Range {'0', '9'}, saw_digit)
		.transition('/', saw_slash)
		.transition('=', saw_eq)
		.transition('!', saw_bang)
		.transition(':', saw_colon)
		.transition('<', saw_lt)
		.transition('>', saw_gt)
		.transition('+', saw_plus)
		.transition('-', saw_dash)
		.transition('*', saw_star)
		.transition('|', saw_pipe)
		.transition('&', saw_amp)
		.transition('^', saw_caret)
	// simple single char tokens
		.transition(';', Semicolon)
		.transition(',', Comma)
		.transition('.', Dot)
		.transition('(', LParen)
		.transition(')', RParen)
		.transition('{', LBrace)
		.transition('}', RBrace)
		.transition('[', LBracket)
		.transition(']', RBracket)
		.transition('~', Tilde)
		.transition('@', At)

	// Identifiers
	.from_state(saw_id_char)
		.default_transition(Identifier)
		.transition('_', saw_id_char)
		.transition(Range {'a', 'z'}, saw_id_char)
		.transition(Range {'A', 'Z'}, saw_id_char)
		.transition(Range {'0', '9'}, saw_id_char)

	// String literals
	.from_state(saw_open_string)
		.default_transition(saw_open_string)
		.transition('"', String)

	// numeric literals
	.from_state(saw_digit)
		.default_transition(Integer)
		.transition(Range {'0', '9'}, saw_digit)
		.transition('.', saw_number_dot)
	.from_state(saw_number_dot)
		.transition(Range {'0', '9'}, saw_decimal_digit)
	.from_state(saw_decimal_digit)
		.default_transition(Number)
		.transition(Range {'0', '9'}, saw_decimal_digit)

	// slash (/) tokens, including comments
	.from_state(saw_slash)
		.default_transition(Slash)
		.transition('=', SlashEq)
		.transition('/', saw_comment_marker)
	.from_state(saw_comment_marker)
		.default_transition(saw_comment_marker)
		.transition('\n', Comment)
		.transition('\0', Comment)

	// equal sign (=) tokens
	.from_state(saw_eq)
		.default_transition(Assign)
		.transition('=', Eq)
		.transition('>', Arrow)

	// bang (!) tokens
	.from_state(saw_bang)
		.default_transition(Bang)
		.transition('=', BangEq)

	// colon (:) tokens
	.from_state(saw_colon)
		.default_transition(Colon)
		.transition('=', ColonEq)
		.transition('>', RPoly)

	// less-than (<) tokens
	.from_state(saw_lt)
		.default_transition(Lt)
		.transition('=', Lte)
		.transition(':', LPoly)

	// greater-than (>) tokens
	.from_state(saw_gt)
		.default_transition(Gt)
		.transition('=', Gte)

	// plus (+) tokens
	.from_state(saw_plus)
		.default_transition(Plus)
		.transition('=', PlusEq)
		.transition('+', PlusPlus)

	// dash (-) tokens
	.from_state(saw_dash)
		.default_transition(Minus)
		.transition('=', MinusEq)
		.transition('-', MinusMinus)

	// start (*) tokens
	.from_state(saw_star)
		.default_transition(Star)
		.transition('=', StarEq)

	// pipe (|) tokens
	.from_state(saw_pipe)
		.default_transition(Pipe)
		.transition('=', PipeEq)
		.transition('|', PipePipe)
		.transition('>', Pizza)

	// amp (&) tokens
	.from_state(saw_amp)
		.default_transition(Amp)
		.transition('=', AmpEq)
		.transition('&', AmpAmp)

	// caret (^) tokens
	.from_state(saw_caret)
		.default_transition(Caret)
		.transition('=', CaretEq)

	;
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
	AutomatonBuilder builder{};
	init_transitions(builder);
	init_offsets(builder.automaton);
	return builder.automaton;
}

} // namespace MainLexer

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

} // namespace EndStates

#define X(name, token_tag, string) TokenTag::token_tag,
constexpr TokenTag token_tags[] = { END_STATES };
#undef X

#define X(name, token_tag, string) string,
static InternedString fixed_strings[] = { END_STATES };
#undef X

#undef END_STATES

constexpr Automaton make() {
	AutomatonBuilder builder{};

	int start_state = builder.new_state();

	add_string(builder, "if", EndStates::If);
	add_string(builder, "for", EndStates::For);
	add_string(builder, "else", EndStates::Else);
	add_string(builder, "fn", EndStates::Fn);
	add_string(builder, "then", EndStates::Then);
	add_string(builder, "return", EndStates::Return);
	add_string(builder, "while", EndStates::While);
	add_string(builder, "match", EndStates::Match);
	add_string(builder, "true", EndStates::True);
	add_string(builder, "false", EndStates::False);
	add_string(builder, "array", EndStates::Array);
	add_string(builder, "null", EndStates::Null);
	add_string(builder, "seq", EndStates::Seq);
	add_string(builder, "tuple", EndStates::Tuple);
	add_string(builder, "struct", EndStates::Struct);
	add_string(builder, "union", EndStates::Union);

	return builder.automaton;
}

} // namespace KeywordLexer

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

	auto eat_whitespace = [&] {
		while (*p == ' ' || *p == '\t' || *p == '\n') {
			p += 1;
		}
	};

	eat_whitespace();
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

		eat_whitespace();
	}
	ta.push_back({TokenTag::END, InternedString()});

	// resolve source locations
	{
		int const real_token_count = ta.size() - 1;
		int code_idx = 0;
		int line = 0;
		int col = 0;

		auto advance_until = [&] (int end_idx) {
			for (; code_idx < end_idx; ++code_idx) {
				if (code_start[code_idx] == '\n') {
					line += 1;
					col = 0;
				} else {
					col += 1;
				}
			}
		};

		for (int i = 0; i < real_token_count; ++i) {
			auto& token = ta.at(i);

			int code_idx1 = token.m_source_location.start.line;
			int code_idx2 = token.m_source_location.end.line;

			advance_until(code_idx1);

			token.m_source_location.start.line = line;
			token.m_source_location.start.col = col;

			advance_until(code_idx2);

			token.m_source_location.end.line = line;
			token.m_source_location.end.col = col;
		}
	}

	return ta;
}
