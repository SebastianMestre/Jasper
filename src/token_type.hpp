#pragma once

/* printable representation */
inline const char* token_type_string[] = {
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

	"KEYWORD_FN",

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

	KEYWORD_FN,

	END,
};
