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

	std::function<void(Core&, int,int)> ll_unify_function;
	std::vector<NodeHeader> ll_node_header;
	std::vector<TermData> ll_term_data;

	bool ll_occurs(int v, int i);

	int ll_find(int i);
	int ll_find_term(int i);
	int ll_find_function(int i);

	void ll_unify(int i, int j);

	int ll_new_var(const char* debug = nullptr);
	int ll_new_term(int f, std::vector<int> args = {}, const char* debug = nullptr);

	bool ll_is_var(int i);
	bool ll_is_term(int i);

	void ll_print_node(int node_header, int d = 0);
};

} // namespace Unification
