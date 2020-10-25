#pragma once

#include "chunked_array.hpp"

#include <type_traits>

template <typename T>
struct SingleAllocator {

	ChunkedArray<T> m_nodes;

	T* make() {
		m_nodes.push_back({});
		return &m_nodes.back();
	}
};

template <typename ...Ts>
struct NodeAllocator;

template <>
struct NodeAllocator<> {

	template <typename U>
	U* make() { static_assert(std::is_same<U, _dummy>::value and "No matching type"); }

	struct _dummy {};
};

template <typename T, typename ...Ts>
struct NodeAllocator<T, Ts...> {

	SingleAllocator<T> m_allocator;
	NodeAllocator<Ts...> m_allocators;

	NodeAllocator() = default;
	NodeAllocator(NodeAllocator&&) = default;
	NodeAllocator(const NodeAllocator&) = delete;

	// it's here
	template <typename U>
	typename std::enable_if<std::is_same<T, U>::value, U*>::type
	make() { return m_allocator.make(); }

	// it's not here
	template <typename U>
	typename std::enable_if<!std::is_same<T, U>::value, U*>::type
	make() { return m_allocators.template make<U>(); }
};
