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

bool MetaUnifier::is_constant_tag(Tag tag) const {
	return tag == Tag::Term || tag == Tag::Mono || tag == Tag::Ctor ||
		tag == Tag::Func;
}

bool MetaUnifier::is_constant(int idx) const {
	return is_constant_tag(tag(idx));
}

bool MetaUnifier::is_singleton_var(int idx) const {
	return is(idx, Tag::Var) && nodes[idx].idx == idx;
}

void MetaUnifier::register_dot_target(int idx) {
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

void MetaUnifier::turn_into(int idx, Tag tag) {
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

void MetaUnifier::turn_dot_result_into(int idx, Tag tag) {
	assert(is(idx, Tag::DotResult));
	assert(tag == Tag::Term || tag == Tag::Ctor);

	nodes[idx].tag = tag;

	if (tag == Tag::Term)
		turn_into(nodes[idx].idx, Tag::Term);

	if (tag == Tag::Ctor)
		turn_into(nodes[idx].idx, Tag::Mono);
}

int MetaUnifier::eval(int idx) {
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

int MetaUnifier::dig_var(int idx) {
	if (!is(idx, Tag::Var))
		return idx;
	if (nodes[idx].idx == idx)
		return idx;
	return nodes[idx].idx = dig_var(nodes[idx].idx);
}

void MetaUnifier::unify(int idx1, int idx2) {
	// FIXME: This code was written when I was very sleepy. Please, check
	//        very carefully, and point out the suspicious bits.


	idx1 = dig_var(idx1);
	idx2 = dig_var(idx2);

	if (!is(idx1, Tag::Var) && is(idx2, Tag::Var)) {
		std::swap(idx1, idx2);
	}

	if (is(idx1, Tag::Var)) {
		// TODO: occurs check
		nodes[idx1].idx = idx2;
	}

	if (is_constant(idx1) && is_constant(idx2)) {
		if (tag(idx1) != tag(idx2))
			Log::fatal() << "bad unification";
		// TODO: maybe turn one into var and point at the other?
		// (so that if one of them becomes dot target, both do)
		return;
	}

	if (is_constant(idx2)) {
		std::swap(idx1, idx2);
	}

	if (is(idx2, Tag::DotResult)) {
		turn_dot_result_into(idx2, tag(idx1));
		// this is probably not ideal
	}

	// TODO: implement unify for two DotResult
}

int MetaUnifier::create_const_node(Tag tag) {
	assert(is_constant_tag(tag));
	int result = nodes.size();
	nodes.push_back({tag, -1, false});
	return result;
}

int MetaUnifier::create_var_node() {
	int result = nodes.size();
	nodes.push_back({Tag::Var, result, false});
	return result;
}

int MetaUnifier::create_dot_node(int target) {
	int result = nodes.size();
	nodes.push_back({Tag::DotResult, target, false});
	register_dot_target(target);
	return result;
}
