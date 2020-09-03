#pragma once

#include <vector>

struct TarjanSolver {
  public:
	TarjanSolver(int vertex_count);
	void add_edge(int u, int v);
	void solve();
	std::vector<std::vector<int>> const& vertices_of_components() const;
	std::vector<int> const& component_of_vertices() const;

  private:
	void visit(int u);

	// inputs
	std::vector<std::vector<int>> m_graph;
	int m_vertex_count;

	// intermediate state
	int m_current_time { 0 };
	// 0 means 'not discovered yet'
	std::vector<int> m_discovery_time;
	std::vector<int> m_lowest_visible;

	// outputs
	std::vector<int> m_component_of;
	// a list of vertices of each strongly connected component
	// they also happen to be in topological order
	std::vector<std::vector<int>> m_scc_vertices;
	std::vector<int> m_vertex_stack;

	bool m_solved { false };
};
