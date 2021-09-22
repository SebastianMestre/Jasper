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
	X(TypeFunctionHandle)                                                      \
	X(MonoTypeHandle)                                                          \
	X(Constructor)                                                             \
	/* All before this point are expressions */                                \
	X(Block)                                                                   \
	X(ReturnStatement)                                                         \
	X(IfElseStatement)                                                         \
	X(WhileStatement)                                                          \
                                                                               \
	X(Program)                                                                 \
	X(Declaration)

#define X(name) #name,
constexpr const char* ast_string[] = {AST_TAGS};
#undef X

#define X(name) name,
enum class ASTTag { AST_TAGS };
#undef X

#undef AST_TAGS
