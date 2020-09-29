#pragma once

#include <vector>
#include <functional>

namespace Unification {

struct Core {

	enum class Tag { Var, Term, };

	struct NodeHeader {
		Tag tag;
		int data_idx;
	};

	struct TermData {
		int function_id; // possibly an external id
		std::vector<int> argument_idx;
	};

	std::function<void(int,int)> unify_function;
	std::vector<NodeHeader> node_header;
	std::vector<TermData> term_data;

	bool occurs(int v, int i);
	int find(int i);
	int find_term(int i);
	int find_function(int i);
	void unify(int i, int j);

	int new_var();
	int new_term(int f, std::vector<int> args);
};

} // namespace Unification
