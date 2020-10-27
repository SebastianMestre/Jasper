#pragma once

#include "ast.hpp"
#include "utils/polymorphic_block_allocator.hpp"
#include "utils/polymorphic_dumb_allocator.hpp"

namespace AST {

/*
// TODO: could we use the AST_TAGS macro here?
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
    ShortFunctionLiteral,
    DeclarationList,
    Declaration,
    Identifier,
    BinaryExpression,
    CallExpression,
    IndexExpression,
    RecordAccessExpression,
    ConstructorExpression,
    Block,
    ReturnStatement,
    IfElseStatement,
    TernaryExpression,
    ForStatement,
    WhileStatement,
    TypeTerm,
    TypeVar,
    UnionExpression,
    TupleExpression,
    StructExpression> {};
	*/

struct Allocator {
	// static constexpr int small_size = 0;
	static constexpr int small_size = 48;
	// static constexpr int small_size = 64 - 8;
	// AutomaticBlockAllocator m_small;
	PolymorphicBlockAllocator<AST> m_small;
	PolymorphicDumbAllocator<AST> m_big;

	Allocator()
	    : m_small(small_size, 4 * 4096)
	    , m_big {4 * 4096} {}

	template<typename T>
	T* make() {
		if (small_size < sizeof(T)) {
			// fprintf(stderr, "big allocation %s\n", __PRETTY_FUNCTION__);
			return m_big.make<T>();
		} else {
			return m_small.make<T>();
		}
	}

};

} // namespace AST
