#pragma once

inline const char* ast_type_string[] = {
	"NumberLiteral",
	"StringLiteral",
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
	"ArgumentList",
	"Block",
	"ReturnStatement",
	"IfStatement",
};

enum class ast_type {
	NumberLiteral,
	StringLiteral,
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
	ArgumentList,
	Block,
	ReturnStatement,
	IfStatement,
};

