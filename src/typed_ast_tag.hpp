#pragma once

#define TYPED_AST_TAGS                                                         \
	X(NumberLiteral)                                                           \
	X(IntegerLiteral)                                                          \
	X(StringLiteral)                                                           \
	X(BooleanLiteral)                                                          \
	X(NullLiteral)                                                             \
	X(ObjectLiteral)                                                           \
	X(ArrayLiteral)                                                            \
	X(DictionaryLiteral)                                                       \
	X(FunctionLiteral)                                                         \
	/* All before this point are literals */                                   \
	X(Identifier)                                                              \
	X(CallExpression)                                                          \
	X(IndexExpression)                                                         \
	X(RecordAccessExpression)                                                  \
	X(TernaryExpression)                                                       \
	X(ConstructorExpression)                                                   \
	/* All before this point are expressions */                                \
	X(Block)                                                                   \
	X(ReturnStatement)                                                         \
	X(IfElseStatement)                                                         \
	X(ForStatement)                                                            \
	X(WhileStatement)                                                          \
                                                                               \
	X(DeclarationList)                                                         \
	X(Declaration)                                                             \
                                                                               \
	X(UnionExpression)                                                         \
	X(StructExpression)                                                        \
	X(TypeTerm)                                                                \
	X(TypeFunctionHandle)                                                      \
	X(MonoTypeHandle)

#define X(name) #name,
constexpr const char* typed_ast_string[] = {TYPED_AST_TAGS};
#undef X

#define X(name) name,
enum class TypedASTTag { TYPED_AST_TAGS };
#undef X

#undef TYPED_AST_TAGS
