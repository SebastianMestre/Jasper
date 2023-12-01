#pragma once

#define AST_EXPR_TAGS                                                          \
	X(NumberLiteral)                                                           \
	X(IntegerLiteral)                                                          \
	X(StringLiteral)                                                           \
	X(BooleanLiteral)                                                          \
	X(NullLiteral)                                                             \
	X(ArrayLiteral)                                                            \
	X(FunctionLiteral)                                                         \
	/* All before this point are literals */                                   \
	X(Identifier)                                                              \
	X(CallExpression)                                                          \
	X(IndexExpression)                                                         \
	X(AccessExpression)                                                        \
	X(MatchExpression)                                                         \
	X(TernaryExpression)                                                       \
	X(ConstructorExpression)                                                   \
	X(SequenceExpression)                                                      \
                                                                               \
	X(UnionExpression)                                                         \
	X(StructExpression)                                                        \
	X(TypeTerm)                                                                \
	X(BuiltinTypeFunction)                                                     \
	X(Constructor)

#define AST_STMT_TAGS                                                          \
	X(Block)                                                                   \
	X(ReturnStatement)                                                         \
	X(IfElseStatement)                                                         \
	X(WhileStatement)                                                          \
	X(ExpressionStatement)                                                     \
	X(Declaration)

namespace AST {

#define X(name) #name,
constexpr const char* expr_string[] = { AST_EXPR_TAGS };
constexpr const char* stmt_string[] = { AST_STMT_TAGS };
#undef X

#define X(name) name,
enum class ExprTag { AST_EXPR_TAGS };
enum class StmtTag { AST_STMT_TAGS };
#undef X

} // namespace AST

#undef AST_TAGS
#undef AST_STMT_TAGS
