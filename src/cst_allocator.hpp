#pragma once

#include "./utils/polymorphic_block_allocator.hpp"
#include "./utils/polymorphic_dumb_allocator.hpp"
#include "cst.hpp"

#include <utility>

namespace CST {

struct Allocator {
	static constexpr int small_size = 48;

	PolymorphicBlockAllocator<CST> m_small;
	PolymorphicDumbAllocator<CST> m_big;

	Allocator()
	    : m_small(small_size, 4 * 4096)
	    , m_big {4 * 4096} {}

	template<typename T, typename ...Args>
	T* make(Args&& ...args) {
		if (small_size < sizeof(T)) {
			return m_big.make<T>(std::forward<Args>(args)...);
		} else {
			return m_small.make<T>(std::forward<Args>(args)...);
		}
	}

};

} // namespace CST
