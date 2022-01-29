#include "meta_unifier.hpp"

#include "log/log.hpp"

#include <iostream>

#include <cassert>

Tag MetaUnifier::tag(int idx) const {
	return nodes[idx].tag;
}

bool MetaUnifier::is(int idx, Tag t) const {
	return tag(idx) == t;
}

bool MetaUnifier::is_ctor(int idx) {
	return is(idx, Tag::Ctor);
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


bool MetaUnifier::occurs(int v, int i){
	assert(is_singleton_var(v));

	i = find(i);

	if (i == v)
		return true;

	return false;
}

int MetaUnifier::find(int idx) {
	if (!is(idx, Tag::Var))
		return idx;
	if (nodes[idx].target == idx)
		return idx;
	return nodes[idx].target = find(nodes[idx].target);
}

void MetaUnifier::turn_into_var(int idx, int target) {
	// make idx be a var that points to target (unsafe)
	assert(find(target) == target);
	nodes[idx].tag = Tag::Var;
	nodes[idx].target = target;
}

bool MetaUnifier::turn_into(int idx, Tag tag) {
	assert(tag != Tag::Var);

	idx = find(idx);
	if (is(idx, Tag::Var)) {
		nodes[idx].tag = tag;
		return true;
	}

	if (this->tag(idx) != tag) {
		char const* str[5] = { "Var", "Term", "Mono", "Ctor", "Func" };

		Log::fatal() << "bad turn_into: turning a " << str[int(this->tag(idx))] << " into a " << str[int(tag)];
	}

	return false;
}

void MetaUnifier::unify(int idx1, int idx2) {
	// FIXME: This code was written when I was very sleepy. Please, check
	//        very carefully, and point out the suspicious bits.

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
			Log::fatal() << "unified different concrete metatypes";
		turn_into_var(idx1, idx2);
		return;
	}

	if (!is_constant(tag1) && is_constant(tag2)) {
		std::swap(idx1, idx2);
		std::swap(tag1, tag2);
	}

	assert(0 && "NOT REACHABLE");
}


int MetaUnifier::eval(int idx) {
	idx = find(idx);

	return idx;
}


void MetaUnifier::make_access_fact(int result, int target) {
	access_facts.push_back({result, target});
}

int MetaUnifier::make_const_node(Tag tag) {
	assert(is_constant(tag));
	int result = nodes.size();
	nodes.push_back({tag, -1});
	return result;
}

int MetaUnifier::make_var_node() {
	int result = nodes.size();
	nodes.push_back({Tag::Var, result});
	return result;
}
