#pragma once

#define AST_TAGS                                                               \
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
                                                                               \
	X(SequenceExpression)                                                      \
	X(UnionExpression)                                                         \
	X(StructExpression)                                                        \
	X(TypeTerm)                                                                \
	X(BuiltinTypeFunction)                                                     \
	X(Constructor)                                                             \
	/* All before this point are expressions */                                \
                                                                               \
	X(Stmt)                                                                    \
	X(Program)

#define AST_STMT_TAGS                                                          \
	X(Block)                                                                   \
	X(ReturnStatement)                                                         \
	X(IfElseStatement)                                                         \
	X(WhileStatement)                                                          \
	X(ExpressionStatement)                                                     \
	X(Declaration)

#define X(name) #name,
constexpr const char* ast_string[] = {AST_TAGS};
constexpr const char* ast_stmt_string[] = {AST_STMT_TAGS};
#undef X

#define X(name) name,
enum class ASTTag { AST_TAGS };
enum class ASTStmtTag { AST_STMT_TAGS };
#undef X

#undef AST_TAGS
#undef AST_STMT_TAGS
