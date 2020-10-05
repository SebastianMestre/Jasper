#pragma once

#include "ast.hpp"

#include <vector>
#include <type_traits>

template <typename T>
struct SingleAllocator {

	std::vector<T> m_nodes;

	T* get(int index) {
		return &m_nodes[index];
	}

	int make() {
		int index = m_nodes.size();
		m_nodes.push_back({});
		return index;
	}
};

template <typename ...Ts>
struct NodeAllocator;

template <>
struct NodeAllocator<> {

	template <typename U>
	U* get(int index) { assert(0 and "No matching type"); }

	template <typename U>
	int make() { assert(0 and "No matching type"); }
};

template <typename T, typename ...Ts>
struct NodeAllocator<T, Ts...> {

	SingleAllocator<T> m_allocator;
	NodeAllocator<Ts...> m_allocators;

	// it's here
	template <typename U>
	typename std::enable_if<std::is_same<T, U>::value, U*>::type
	get(int index) { return m_allocator.get(index); }

	template <typename U>
	typename std::enable_if<std::is_same<T, U>::value, int>::type
	make() { return m_allocator.make(); }

	// it's not here
	template <typename U>
	typename std::enable_if<!std::is_same<T, U>::value, U*>::type
	get(int index) { return m_allocators.template get<U>(index); }

	template <typename U>
	typename std::enable_if<!std::is_same<T, U>::value, int>::type
	make() { return m_allocators.template make<U>(); }
};
