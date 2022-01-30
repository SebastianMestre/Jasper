#pragma once

#include <vector>

struct UnionFind {
	int new_var() {
		int result = repr.size();
		repr.push_back(result);
		return result;
	}

	int find(int i) {
		if (repr[i] == i)
			return i;
		return repr[i] = find(repr[i]);
	}

	void join_left_to_right(int i, int j) {
		repr[find(i)] = find(j);
	}

	bool is_single(int i) {
		return i == repr[i];
	}

  private:
	std::vector<int> repr;
};
