#pragma once

#define TOKEN_TAGS                                                             \
	X(ADD, "+")                                                                \
	X(SUB, "-")                                                                \
	X(DIV, "/")                                                                \
	X(MUL, "*")                                                                \
	X(AND, "&")                                                                \
	X(IOR, "|")                                                                \
	X(XOR, "^")                                                                \
	X(LT, "<")                                                                 \
	X(GT, ">")                                                                 \
	X(LTE, "<=")                                                               \
	X(GTE, ">=")                                                               \
	X(COMPL, "COMPL")                                                          \
	X(LOGIC_COMPL, "LOGIC_COMPL")                                              \
	X(LOGIC_AND, "LOGIC_AND")                                                  \
	X(LOGIC_IOR, "LOGIC_IOR")                                                  \
	X(EQUAL, "==")                                                             \
	X(NOT_EQUAL, "!=")                                                         \
	X(ASSIGN, "=")                                                             \
	X(DECLARE_ASSIGN, ":=")                                                    \
	X(DECLARE, ":")                                                            \
	X(INCREMENT, "INCREMENT")                                                  \
	X(DECREMENT, "DECREMENT")                                                  \
	X(ADD_TO, "ADD_TO")                                                        \
	X(SUB_TO, "SUB_TO")                                                        \
	X(MUL_TO, "MUL_TO")                                                        \
	X(DIV_TO, "DIV_TO")                                                        \
	X(AND_TO, "AND_TO")                                                        \
	X(IOR_TO, "IOR_TO")                                                        \
	X(XOR_TO, "XOR_TO")                                                        \
	X(AT, "AT")                                                                \
	X(DOT, "DOT")                                                              \
	X(COMMA, "COMMA")                                                          \
	X(PIZZA, "PIZZA")                                                          \
	X(ARROW, "ARROW")                                                          \
	X(BRACE_OPEN, "BRACE_OPEN")                                                \
	X(BRACKET_OPEN, "BRACKET_OPEN")                                            \
	X(PAREN_OPEN, "PAREN_OPEN")                                                \
	X(POLY_OPEN, "POLY_OPEN")                                                  \
	X(BRACE_CLOSE, "BRACE_CLOSE")                                              \
	X(BRACKET_CLOSE, "BRACKET_CLOSE")                                          \
	X(PAREN_CLOSE, "PAREN_CLOSE")                                              \
	X(POLY_CLOSE, "POLY_CLOSE")                                                \
	X(SEMICOLON, "SEMICOLON")                                                  \
	X(COLON, "COLON")                                                          \
	X(NUMBER, "NUMBER")                                                        \
	X(INTEGER, "INTEGER")                                                      \
	X(STRING, "STRING")                                                        \
	X(KEYWORD_FALSE, "KEYWORD_FALSE")                                          \
	X(KEYWORD_TRUE, "KEYWORD_TRUE")                                            \
	X(KEYWORD_NULL, "KEYWORD_NULL")                                            \
	X(IDENTIFIER, "IDENTIFIER")                                                \
	X(KEYWORD, "KEYWORD")                                                      \
	X(KEYWORD_FN, "KEYWORD_FN")                                                \
	X(KEYWORD_ARRAY, "KEYWORD_ARRAY")                                          \
	X(KEYWORD_RETURN, "KEYWORD_RETURN")                                        \
	X(KEYWORD_IF, "KEYWORD_IF")                                                \
	X(KEYWORD_THEN, "KEYWORD_THEN")                                            \
	X(KEYWORD_ELSE, "KEYWORD_ELSE")                                            \
	X(KEYWORD_FOR, "KEYWORD_FOR")                                              \
	X(KEYWORD_WHILE, "KEYWORD_WHILE")                                          \
	X(KEYWORD_MATCH, "KEYWORD_MATCH")                                          \
	X(KEYWORD_SEQ, "KEYWORD_SEQ")                                              \
	X(KEYWORD_UNION, "KEYWORD_UNION")                                          \
	X(KEYWORD_TUPLE, "KEYWORD_TUPLE")                                          \
	X(KEYWORD_STRUCT, "KEYWORD_STRUCT")                                        \
	X(END, "(end of file)")

/* printable representation */
#define X(name, str) str,
constexpr const char* token_string[] = {TOKEN_TAGS};
#undef X

/* internal representation */
#define X(name, str) name,
enum class TokenTag { TOKEN_TAGS };
#undef X

#undef TOKEN_TAGS
