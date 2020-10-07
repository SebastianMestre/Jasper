#pragma once

#define AST_TAGS \
	X(NumberLiteral) \
	X(IntegerLiteral) \
	X(StringLiteral) \
	X(BooleanLiteral) \
	X(NullLiteral) \
	X(ObjectLiteral) \
	X(ArrayLiteral) \
	X(DictionaryLiteral) \
	X(FunctionLiteral) \
	X(ShortFunctionLiteral) \
 \
	X(DeclarationList) \
	X(Declaration) \
	X(Identifier) \
	X(BinaryExpression) \
	X(CallExpression) \
	X(IndexExpression) \
	X(RecordAccessExpression) \
	X(Block) \
	X(ReturnStatement) \
	X(IfElseStatement) \
	X(TernaryExpression) \
	X(ForStatement) \
	X(WhileStatement) \
\
	X(TypeTerm) \
	X(TypeVar) \
	X(UnionExpression) \
	X(TupleExpression) \
	X(StructExpression)

#define X(name) #name,
constexpr const char* ast_string[] = {
	AST_TAGS
};
#undef X

#define X(name) name,
enum class ASTTag {
	AST_TAGS
};
#undef X

#undef AST_TAGS
