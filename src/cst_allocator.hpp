#pragma once

#include "cst.hpp"
#include "utils/polymorphic_block_allocator.hpp"
#include "utils/polymorphic_dumb_allocator.hpp"

namespace CST {

struct Allocator {
	static constexpr int small_size = 48;

	PolymorphicBlockAllocator<CST> m_small;
	PolymorphicDumbAllocator<CST> m_big;

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

} // namespace CST
