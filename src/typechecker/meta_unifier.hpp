#pragma once

#include <vector>

namespace AST {
struct Declaration;
}

enum class Tag {
	Var,  // computed
	Term, // constant
	Mono, // constant
	Ctor, // constant
	Func, // constant
};

struct Node {
	Tag tag;
	int target;
};

struct AccessFact {
	int result;
	int target;
};

struct CtorFact {
	int target;
};

struct MetaUnifier {
	// TODO: handle dot targets

	Tag tag(int idx) const;

	bool is(int idx, Tag t) const;
	bool is_ctor(int idx);
	bool is_singleton_var(int idx) const;
	bool is_constant(Tag tag) const;
	bool is_constant(int idx) const;

	bool turn_into(int idx, Tag tag);
	int eval(int idx);
	void unify(int idx1, int idx2);

	int make_const_node(Tag tag);
	int make_var_node();

	void make_access_fact(int result, int target);
	void make_ctor_fact(int target);

	void solve();

private:
	bool occurs(int var, int target);
	int find(int idx);

	void turn_into_var(int idx, int target);

	std::vector<Node> nodes;
	std::vector<AccessFact> access_facts;
	std::vector<CtorFact> ctor_facts;
public:
	std::vector<std::vector<AST::Declaration*>> const* comp {nullptr};
};


