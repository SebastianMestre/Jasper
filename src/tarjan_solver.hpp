#pragma once

#include <vector>

struct TarjanSolver {
public:
	TarjanSolver(int vertex_count);
	void add_adge(int u, int v);
	void solve();
private:
	void visit(int u);

	// inputs
	int m_vertex_count;
	std::vector<std::vector<int>> m_graph;

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
};
