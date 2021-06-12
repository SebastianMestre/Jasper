#pragma once

#include <vector>

namespace AST {
struct Declaration;
}

enum class Tag {
	Var,       // computed
	DotResult, // computed
	Term, // constant
	Mono, // constant
	Ctor, // constant
	Func, // constant
};

struct Node {
	Tag tag;
	int idx;
	bool is_dot_target{false};
};

struct MetaUnifier {
	// TODO: handle dot targets

	std::vector<std::vector<AST::Declaration*>> const* comp {nullptr};
	std::vector<Node> nodes;

	Tag tag(int idx) const;
	bool is_dot_target(int idx) const;

	bool is(int idx, Tag t) const;
	bool is_singleton_var(int idx) const;
	bool is_constant(Tag tag) const;
	bool is_constant(int idx) const;

	void register_dot_target(int idx);
	void turn_into(int idx, Tag tag);
	void turn_dot_result_into(int idx, Tag tag);
	int eval(int idx);
	int dig_var(int idx);
	void unify(int idx1, int idx2);

	int create_const_node(Tag tag);
	int create_var_node();
	int create_dot_node(int target);

private:
	void turn_into_var(int idx, int target);
};


