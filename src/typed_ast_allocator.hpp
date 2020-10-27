#pragma once

#include "typed_ast.hpp"
#include "utils/polymorphic_block_allocator.hpp"
#include "utils/polymorphic_dumb_allocator.hpp"

namespace TypedAST {

struct Allocator {
	static constexpr int small_size = 56;
	PolymorphicBlockAllocator<TypedAST> m_small;
	PolymorphicDumbAllocator<TypedAST> m_big;

	Allocator()
	    : m_small(small_size, 4 * 4096)
	    , m_big {4 * 4096} {}

	template<typename T>
	T* make() {
		if (small_size < sizeof(T)) {
			return m_big.make<T>();
		} else {
			return m_small.make<T>();
		}
	}
};

} // namespace TypedAST
