#pragma once

#include <vector>

#include <cassert>

struct Node {
	int target;
};

struct Nodes {
	std::vector<Node> nodes;

	// ENSURES: return value is a singleton variable
	int find(int idx) {
		if (nodes[idx].target == idx)
			return idx;
		return nodes[idx].target = find(nodes[idx].target);
	}

	void point_at(int idx1, int idx2) {
		assert(idx1 == find(idx1));
		nodes[idx1].target = idx2;
	}

	int create_node() {
		int result = nodes.size();
		nodes.push_back({result});
		return result;
	}
};

