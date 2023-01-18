#pragma once

#define CST_TAGS                                                               \
	X(NumberLiteral)                                                           \
	X(IntegerLiteral)                                                          \
	X(StringLiteral)                                                           \
	X(BooleanLiteral)                                                          \
	X(NullLiteral)                                                             \
	X(ArrayLiteral)                                                            \
	X(BlockFunctionLiteral)                                                    \
	X(FunctionLiteral)                                                         \
                                                                               \
	X(Program)                                                                 \
                                                                               \
	X(Identifier)                                                              \
	X(BinaryExpression)                                                        \
	X(CallExpression)                                                          \
	X(IndexExpression)                                                         \
	X(AccessExpression)                                                        \
	X(MatchExpression)                                                         \
	X(ConstructorExpression)                                                   \
	X(TernaryExpression)                                                       \
	X(SequenceExpression)                                                      \
                                                                               \
	X(PlainDeclaration)                                                        \
	X(FuncDeclaration)                                                         \
	X(BlockFuncDeclaration)                                                    \
	X(Block)                                                                   \
	X(ReturnStatement)                                                         \
	X(IfElseStatement)                                                         \
	X(ForStatement)                                                            \
	X(WhileStatement)                                                          \
	X(ExpressionStatement)                                                     \
                                                                               \
	X(TypeTerm)                                                                \
	X(TypeVar)                                                                 \
	X(UnionExpression)                                                         \
	X(StructExpression)

#define X(name) #name,
constexpr const char* cst_string[] = { CST_TAGS };
#undef X

#define X(name) name,
enum class CSTTag { CST_TAGS };
#undef X

#undef CST_TAGS
