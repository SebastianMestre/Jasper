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
	"ShortFunctionLiteral",

	"DeclarationList",
	"Declaration",
	"Identifier",
	"BinaryExpression",
	"CallExpression",
	"IndexExpression",
	"Block",
	"ReturnStatement",
	"IfElseStatement",
	"TernaryExpression",
	"ForStatement",
	"WhileStatement",

	"TypeTerm",
};

enum class ASTTag {
	NumberLiteral,
	IntegerLiteral,
	StringLiteral,
	BooleanLiteral,
	NullLiteral,
	ObjectLiteral,
	ArrayLiteral,
	DictionaryLiteral,
	FunctionLiteral,
	ShortFunctionLiteral,

	DeclarationList,
	Declaration,
	Identifier,
	BinaryExpression,
	CallExpression,
	IndexExpression,
	Block,
	ReturnStatement,
	IfElseStatement,
	TernaryExpression,
	ForStatement,
	WhileStatement,

	TypeTerm,
};
