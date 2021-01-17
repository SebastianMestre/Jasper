#pragma once

#define AST_TAGS                                                               \
	X(NumberLiteral)                                                           \
	X(IntegerLiteral)                                                          \
	X(StringLiteral)                                                           \
	X(BooleanLiteral)                                                          \
	X(NullLiteral)                                                             \
	X(ArrayLiteral)                                                            \
	X(DictionaryLiteral)                                                       \
	X(BlockFunctionLiteral)                                                    \
	X(FunctionLiteral)                                                         \
                                                                               \
	X(DeclarationList)                                                         \
	X(Declaration)                                                             \
	X(Identifier)                                                              \
	X(BinaryExpression)                                                        \
	X(CallExpression)                                                          \
	X(IndexExpression)                                                         \
	X(AccessExpression)                                                        \
	X(MatchExpression)                                                         \
	X(ConstructorExpression)                                                   \
	X(SequenceExpression)                                                      \
                                                                               \
	X(Block)                                                                   \
	X(ReturnStatement)                                                         \
	X(IfElseStatement)                                                         \
	X(IfElseExpression)                                                       \
	X(ForStatement)                                                            \
	X(WhileStatement)                                                          \
                                                                               \
	X(TypeTerm)                                                                \
	X(TypeVar)                                                                 \
	X(UnionExpression)                                                         \
	X(TupleExpression)                                                         \
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
