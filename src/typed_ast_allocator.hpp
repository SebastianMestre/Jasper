#pragma once

#include "typed_ast.hpp"
// #include "utils/node_allocator.hpp"
#include "utils/automatic_block_allocator.hpp"

namespace TypedAST {

/*
// TODO: could we use the TYPED_AST_TAGS macro here?
struct Allocator : public NodeAllocator<
    NumberLiteral,
    IntegerLiteral,
    StringLiteral,
    BooleanLiteral,
    NullLiteral,
    ObjectLiteral,
    ArrayLiteral,
    DictionaryLiteral,
    FunctionLiteral,
    Identifier,
    CallExpression,
    IndexExpression,
    RecordAccessExpression,
    TernaryExpression,
    ConstructorExpression,
    Block,
    ReturnStatement,
    IfElseStatement,
    ForStatement,
    WhileStatement,
    DeclarationList,
    Declaration,
    StructExpression,
    TypeTerm,
    TypeFunctionHandle,
    MonoTypeHandle> {};
	*/

struct Allocator {
	// static constexpr int small_size = 64 - 8;
	static constexpr int small_size = 64 - 8;
	AutomaticBlockAllocator m_small;

	Allocator() : m_small(small_size, 4*4096) {}

	template<typename T>
	T* make() {
		if (small_size < sizeof(T)) {
			// fprintf(stderr, "big allocation %s\n", __PRETTY_FUNCTION__);
			return new T;
		} else {
			return m_small.make<T>();
		}
	}
};

} // namespace TypedAST
