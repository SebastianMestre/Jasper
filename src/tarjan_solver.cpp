#include "tarjan_solver.hpp"

#include <algorithm>
#include <limits>

#include <cassert>

TarjanSolver::TarjanSolver(int vertex_count)
    : m_graph(vertex_count)
    , m_vertex_count {vertex_count}
    , m_discovery_time(vertex_count, 0)
    , m_lowest_visible(vertex_count, std::numeric_limits<int>::max())
    , m_component_of(vertex_count, -1) {}

void TarjanSolver::add_edge(int u, int v) {
	assert(!m_solved);
	assert(0 <= u && u < m_vertex_count);
	assert(0 <= v && v < m_vertex_count);
	m_graph[u].push_back(v);
}

void TarjanSolver::solve() {
	assert(!m_solved);

	for (int u {0}; u < m_vertex_count; ++u) {
		if (!m_discovery_time[u]) {
			visit(u);
		}
	}

	m_solved = true;
}

void TarjanSolver::visit(int u) {
	assert(!m_solved);
	assert(0 <= u && u < m_vertex_count);

	m_current_time += 1;
	m_discovery_time[u] = m_current_time;
	m_lowest_visible[u] = m_current_time;
	m_vertex_stack.push_back(u);

	for (int v : m_graph[u]) {
		if (!m_discovery_time[v]) {
			// v is not yet discovered
			visit(v);
			m_lowest_visible[u] =
			    std::min(m_lowest_visible[u], m_lowest_visible[v]);
		} else if (m_component_of[v] == -1) {
			// v is discovered, but not yet assigned to a
			// component. (i.e. v is still in the stack)
			m_lowest_visible[u] =
			    std::min(m_lowest_visible[u], m_lowest_visible[v]);
		}
	}

	// there are no edges going over `u`, thus it must be
	// the root of an SCC.
	// We don't really care about roots, so we use an
	// index, instead.
	if (m_lowest_visible[u] == m_discovery_time[u]) {
		int component_number = m_scc_vertices.size();
		m_scc_vertices.push_back({});
		while (1) {
			int w = m_vertex_stack.back();
			m_vertex_stack.pop_back();
			m_component_of[w] = component_number;
			m_scc_vertices.back().push_back(w);
			if (w == u)
				break;
		}
	}
}

std::vector<std::vector<int>> const& TarjanSolver::vertices_of_components() const {
	assert(m_solved);
	return m_scc_vertices;
}

std::vector<int> const& TarjanSolver::component_of_vertices() const {
	assert(m_solved);
	return m_component_of;
}
