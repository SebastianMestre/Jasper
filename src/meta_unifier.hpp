#pragma once

#include <vector>
#include <cassert>

#include <iostream>

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

	bool is(int idx, Tag tag) const {
		return nodes[idx].tag == tag;
	}

	bool is_dot_target(int idx) const {
		return nodes[idx].is_dot_target;
	}

	bool is_singleton_var(int idx) const {
		return is(idx, Tag::Var) && nodes[idx].idx == idx;
	}

	bool is_constant_tag(Tag tag) const {
		return tag == Tag::Term || tag == Tag::Mono || tag == Tag::Ctor ||
		       tag == Tag::Func;
	}

	bool is_constant(int idx) const {
		return is_constant_tag(tag(idx));
	}

	Tag tag(int idx) const {
		return nodes[idx].tag;
	}

	void register_dot_target(int idx) {
		// TODO: maybe assign dot target to every intermediate step if var?
		idx = dig_var(idx);

		if (is(idx, Tag::Var)) {
			nodes[idx].is_dot_target = true;
			return;
		}

		if (is(idx, Tag::DotResult)) {
			nodes[idx].is_dot_target = true;
			turn_dot_result_into(idx, Tag::Term);
			return;
		}

		if (is(idx, Tag::Mono)) {
			nodes[idx].is_dot_target = true;
			return;
		}

		if (is(idx, Tag::Term)) {
			nodes[idx].is_dot_target = true;
			return;
		}

		assert(0);
	}

	void turn_into(int idx, Tag tag) {
		assert(tag != Tag::DotResult);
		assert(tag != Tag::Var);

		idx = dig_var(idx);
		if (is(idx, Tag::Var)) { nodes[idx].tag = tag;
			return;
		}

		if (is(idx, Tag::DotResult)) {
			return turn_dot_result_into(idx, tag);
		}

		assert(nodes[idx].tag == tag);
	}

	void turn_dot_result_into(int idx, Tag tag) {
		assert(is(idx, Tag::DotResult));
		assert(tag == Tag::Term || tag == Tag::Ctor);

		nodes[idx].tag = tag;

		if (tag == Tag::Term)
			turn_into(nodes[idx].idx, Tag::Term);

		if (tag == Tag::Ctor)
			turn_into(nodes[idx].idx, Tag::Mono);
	}

	int eval(int idx) {
		idx = dig_var(idx);

		if (!is(idx, Tag::DotResult))
			return idx;

		int target = dig_var(nodes[idx].idx);

		if (is(target, Tag::Term) || is(target, Tag::DotResult) || is_dot_target(idx))
			turn_dot_result_into(idx, Tag::Term);

		if (is(target, Tag::Mono))
			turn_dot_result_into(idx, Tag::Ctor);

		return idx;
	}

	int dig_var(int idx) {
		if (!is(idx, Tag::Var))
			return idx;
		if (nodes[idx].idx == idx)
			return idx;
		return nodes[idx].idx = dig_var(nodes[idx].idx);
	}

	int create_const_node(Tag tag) {
		assert(is_constant_tag(tag));
		int result = nodes.size();
		nodes.push_back({tag, -1, false});
		std::cerr << "MetaUnifier::create_const_node() => " << result << "\n";
		return result;
	}

	int create_var_node() {
		int result = nodes.size();
		nodes.push_back({Tag::Var, result, false});
		return result;
	}

	int create_dot_node(int target) {
		int result = nodes.size();
		nodes.push_back({Tag::DotResult, target, false});
		register_dot_target(target);
		return result;
	}
};


