#pragma once

constexpr const char* ast_type_string[] = {
	"NumberLiteral",
	"IntegerLiteral",
	"StringLiteral",
	"BooleanLiteral",
	"NullLiteral",
	"ObjectLiteral",
	"ArrayLiteral",
	"DictionaryLiteral",
	"FunctionLiteral",

	"DeclarationList",
	"Declaration",
	"Identifier",
	"BinaryExpression",
	"CallExpression",
	"IndexExpression",
	"Block",
	"ReturnStatement",
	"IfStatement",
	"ForStatement",

	"TypeTerm",
};

enum class ast_type {
	NumberLiteral,
	IntegerLiteral,
	StringLiteral,
	BooleanLiteral,
	NullLiteral,
	ObjectLiteral,
	ArrayLiteral,
	DictionaryLiteral,
	FunctionLiteral,

	DeclarationList,
	Declaration,
	Identifier,
	BinaryExpression,
	CallExpression,
	IndexExpression,
	Block,
	ReturnStatement,
	IfStatement,
	ForStatement,

	TypeTerm,
};

