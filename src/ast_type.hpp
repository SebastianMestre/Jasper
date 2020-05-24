#pragma once

constexpr const char* ast_type_string[] = {
	"NumberLiteral",
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
};

enum class ast_type {
	NumberLiteral,
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
};

