
struct KorasajuSolver {
private:
	vector<bool> visited;
	vector<vector<int>> graph;
	vector<vector<int>> transpose_graph;
	vector<int> component_of_vertex;

	vector<vector<int>> component_vertices;
	vector<set<int>> component_graph;

	vector<int> vertex_order;

	int new_component () {
		component_graph.push_back({});
		component_vertices.push_back({});
		return component_vertices.size();
	}

	void visit(int u) {
		if(visited[u])
			return;

		visited[u] = true;
		for (int v : graph[u]){
			visit(v);
			vertex_order.push_back(u);
		}
	}

	void assign(int u, int root) {
		if(component_of_vertex[u] == -1)
			return;

		component_of_vertex[u] = root;
		for (int v : transpose_graph[u])
			dfs2(v, root);
	}

public:
	void solve () {
		for(int u = 0; u != n; ++u)
			visit(u);

		reverse(vertex_order.begin(), vertex_order.end());

		component_of_vertex.assign(n, -1);
		for(int i = 0; i != n; ++i){
			int u = vertex_order[i];
			assign(u, new_component());
		}

		for(int u = 0; u != n; ++u){
			int source_component = component_of_vertex[u];
			for(int v : graph[u]){
				int destination_component = component_of_vertex[v];
				if(source_component == destination_component)
					continue;

				component_graph[source_component].insert(destination_component);
			}
		}
	}
};

