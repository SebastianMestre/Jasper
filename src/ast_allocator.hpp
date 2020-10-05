#pragma once

#include "ast.hpp"

#include <vector>
#include <type_traits>

namespace AST {

template <typename ...Ts>
struct ASTAllocator;

// base case
template <typename T>
struct ASTAllocator<T> {

	std::vector<T> m_nodes;

	template <typename U>
	U* get(int index) {
		// has to be here, otherwise error
		static_assert(std::is_same<T, U>::value, "requested non-existent node");
		return &m_nodes[index];
	}

	template <typename U>
	int make() {
		int index = m_nodes.size();
		m_nodes.push_back({});
		return index;
	}
};

template <typename T, typename ...Ts>
struct ASTAllocator<T, Ts...> {

	std::vector<T> m_nodes;
	ASTAllocator<Ts...> m_allocators;

	// it's here
	template <typename U>
	typename std::enable_if<std::is_same<T, U>::value, U*>::type
	get(int index) {
		return &m_nodes[index];
	}

	template <typename U>
	typename std::enable_if<std::is_same<T, U>::value, int>::type
	make() {
		int index = m_nodes.size();
		m_nodes.push_back({});
		return index;
	}

	// it's not here
	template <typename U>
	typename std::enable_if<!std::is_same<T, U>::value, U*>::type
	get(int index) {
		return m_allocators.template get<U>(index);
	}

	template <typename U>
	typename std::enable_if<!std::is_same<T, U>::value, int>::type
	make() {
		return m_allocators.template make<U>();
	}
};

} // namespace AST
