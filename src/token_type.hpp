#pragma once

/* printable representation */
constexpr const char* token_type_string[] = {
    "+",
    "-",
    "/",
    "*",
    "&",
    "|",
    "^",

    "<",
    ">",
    "<=",
    ">=",

    "COMPL",

    "LOGIC_COMPL",
    "LOGIC_AND",
    "LOGIC_IOR",

    "==",
    "!=",

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

    "DOT",
    "COMMA",
    "PIZZA",
    "ARROW",

    "BRACE_OPEN",
    "BRACKET_OPEN",
    "PAREN_OPEN",
    "POLY_OPEN",

    "BRACE_CLOSE",
    "BRACKET_CLOSE",
    "PAREN_CLOSE",
    "POLY_CLOSE",

    "SEMICOLON",
    "COLON",

    "NUMBER",
    "INTEGER",
    "STRING",
    "KEYWORD_FALSE",
    "KEYWORD_TRUE",
    "KEYWORD_NULL",
    "IDENTIFIER",
    "KEYWORD",

    "KEYWORD_FN",
    "KEYWORD_DICT",
    "KEYWORD_OBJECT",
    "KEYWORD_ARRAY",
    "KEYWORD_RETURN",
    "KEYWORD_IF",
    "KEYWORD_THEN",
    "KEYWORD_ELSE",
    "KEYWORD_FOR",
    "KEYWORD_WHILE",

    "END",
};

/* internal representation */
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

	DOT,
	COMMA,
	PIZZA,
	ARROW,

	BRACE_OPEN,
	BRACKET_OPEN,
	PAREN_OPEN,
	POLY_OPEN,

	BRACE_CLOSE,
	BRACKET_CLOSE,
	PAREN_CLOSE,
	POLY_CLOSE,

	SEMICOLON,
	COLON,

	NUMBER,
	INTEGER,
	STRING,
	KEYWORD_FALSE,
	KEYWORD_TRUE,
	KEYWORD_NULL,
	IDENTIFIER,
	KEYWORD,

	KEYWORD_FN,
	KEYWORD_DICT,
	KEYWORD_OBJECT,
	KEYWORD_ARRAY,
	KEYWORD_RETURN,
	KEYWORD_IF,
	KEYWORD_THEN,
	KEYWORD_ELSE,
	KEYWORD_FOR,
	KEYWORD_WHILE,

	END,
};
