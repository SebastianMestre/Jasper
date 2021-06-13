#include "meta_unifier.hpp"

#include "log/log.hpp"

#include <iostream>

#include <cassert>

Tag MetaUnifier::tag(int idx) const {
	return nodes[idx].tag;
}

bool MetaUnifier::is_dot_target(int idx) const {
	return nodes[idx].is_dot_target;
}

bool MetaUnifier::is(int idx, Tag t) const {
	return tag(idx) == t;
}

bool MetaUnifier::is_constant(Tag t) const {
	return t == Tag::Term || t == Tag::Mono || t == Tag::Ctor || t == Tag::Func;
}

bool MetaUnifier::is_constant(int idx) const {
	return is_constant(tag(idx));
}

bool MetaUnifier::is_singleton_var(int idx) const {
	return is(idx, Tag::Var) && nodes[idx].target == idx;
}

// make idx be a var that points to target (unsafe)
void MetaUnifier::turn_into_var(int idx, int target) {
	// TODO: propagate is_dot_target
	assert(find(target) == target);
	nodes[idx].tag = Tag::Var;
	nodes[idx].target = target;
}

void MetaUnifier::register_dot_target(int idx) {
	idx = find(idx);

	if (is(idx, Tag::Ctor) || is(idx, Tag::Func))
		Log::fatal() << "used dot operator on a constructor or typefunc";

	nodes[idx].is_dot_target = true;
}

void MetaUnifier::turn_into(int idx, Tag tag) {
	assert(tag != Tag::DotResult);
	assert(tag != Tag::Var);

	idx = find(idx);
	if (is(idx, Tag::Var)) {
		nodes[idx].tag = tag;
		return;
	}

	if (is(idx, Tag::DotResult)) {
		return turn_dot_result_into(idx, tag);
	}

	assert(nodes[idx].tag == tag);
}

void MetaUnifier::turn_dot_result_into(int idx, Tag tag) {
	assert(is(idx, Tag::DotResult));
	assert(tag == Tag::Term || tag == Tag::Ctor);

	nodes[idx].tag = tag;

	if (tag == Tag::Term)
		turn_into(nodes[idx].target, Tag::Term);

	if (tag == Tag::Ctor)
		turn_into(nodes[idx].target, Tag::Mono);
}

int MetaUnifier::eval(int idx) {
	idx = find(idx);

	if (!is(idx, Tag::DotResult))
		return idx;

	int target = find(nodes[idx].target);

	if (is(target, Tag::Term) || is(target, Tag::DotResult) || is_dot_target(idx))
		turn_dot_result_into(idx, Tag::Term);

	if (is(target, Tag::Mono))
		turn_dot_result_into(idx, Tag::Ctor);

	return idx;
}

int MetaUnifier::find(int idx) {
	if (!is(idx, Tag::Var))
		return idx;
	if (nodes[idx].target == idx)
		return idx;
	return nodes[idx].target = find(nodes[idx].target);
}

void MetaUnifier::unify(int idx1, int idx2) {
	// FIXME: This code was written when I was very sleepy. Please, check
	//        very carefully, and point out the suspicious bits.

	// TODO: propagate is_dot_target
	// TODO: occurs check

	idx1 = find(idx1);
	idx2 = find(idx2);

	auto tag1 = tag(idx1);
	auto tag2 = tag(idx2);

	if (tag1 != Tag::Var && tag2 == Tag::Var) {
		std::swap(idx1, idx2);
		std::swap(tag1, tag2);
	}

	if (tag1 == Tag::Var) {
		turn_into_var(idx1, idx2);
		return;
	}

	if (is_constant(tag1) && is_constant(tag2)) {
		if (tag1 != tag2)
			Log::fatal() << "bad unification";
		turn_into_var(idx1, idx2);
		return;
	}

	if (!is_constant(tag1) && is_constant(tag2)) {
		std::swap(idx1, idx2);
		std::swap(tag1, tag2);
	}

	if (is_constant(tag1) && tag2 == Tag::DotResult) {
		turn_dot_result_into(idx2, tag1);
		turn_into_var(idx1, idx2);
		return;
	}

	if (tag1 == Tag::DotResult && tag2 == Tag::DotResult) {
		int target1 = nodes[idx1].target;
		int target2 = nodes[idx2].target;

		unify(target1, target2);
		turn_into_var(idx1, idx2);
	}

	assert(0 && "NOT REACHABLE");
}

int MetaUnifier::make_const_node(Tag tag) {
	assert(is_constant(tag));
	int result = nodes.size();
	nodes.push_back({tag, -1, false});
	return result;
}

int MetaUnifier::make_var_node() {
	int result = nodes.size();
	nodes.push_back({Tag::Var, result, false});
	return result;
}

int MetaUnifier::make_dot_node(int target) {
	int result = nodes.size();
	nodes.push_back({Tag::DotResult, target, false});
	register_dot_target(target);
	return result;
}
