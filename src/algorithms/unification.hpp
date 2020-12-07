#pragma once

#include <vector>
#include <functional>

namespace Unification {

struct Core {

	enum class Tag { Var, Term, };

	struct NodeHeader {
		Tag tag;
		int data_idx;
		const char* debug {nullptr};
	};

	struct TermData {
		int function_id; // external id
		std::vector<int> argument_idx;
	};

	std::function<void(Core&, int,int)> unify_function;
	std::vector<NodeHeader> node_header;
	std::vector<TermData> term_data;

	bool occurs(int v, int i);

	int find(int i);
	int find_term(int i);
	int find_function(int i);

	void unify(int i, int j);

	int new_var(const char* debug = nullptr);
	int new_term(int f, std::vector<int> args = {}, const char* debug = nullptr);

	bool is_var(int i);
	bool is_term(int i);

	void print_node(int node_header, int d = 0);
};

} // namespace Unification
