#pragma once

#include <cassert>

enum class ast_vtype {
	Null,

	Integer,
	Float,
	Boolean,
	String,
	Error,

	Array,
	Object,
	Dictionary,

	Function,
	NativeFunction,

	TypeError,
	Void,
	Undefined,
	Wildcard
};

namespace TypeChecker {

struct Type {
	virtual bool is_var() const { return false; }
};

struct Term : Type {
	// TODO: replace name with a TypeDescriptor* or somth
	// We eventually need to know what the type actually is.
	int name;
	std::vector<Type*> args;
};

struct Var : Type {
	Type* instance { nullptr };
	bool is_var() const override { return true; }
};

// TODO: move implementations to .cpp file

// Basically find with path compression on disjoint set
// TODO: maybe decouple the data structure from the logic?
inline Type* prune(Type* t) {
	assert(t);

	if (!t->is_var())
		return t;

	auto vt = static_cast<Var*>(t);

	if (!vt->instance)
		return vt;

	return vt->instance = prune(vt->instance);
}

// v must be prunned before passing
inline bool occurs_in(Var* v, Type* t) {
	auto tt = prune(t);

	if (v == tt)
		return true;
	if (tt->is_var())
		return false;

	auto ttt = static_cast<Term*>(tt);
	for (Type* c : ttt->args)
		if (occurs_in(v, c))
			return true;

	return false;
}

// a must be prunned before passing
inline void unify(Type* a, Type* b) {
	if (a->is_var()) {
		if (a != b) {
			auto va = static_cast<Var*>(a);
			assert(!occurs_in(va, b));
			va->instance = b;
		}
	} else if (b->is_var()) {
		return unify(b, a);
	} else {
		auto ta = static_cast<Term*>(a);
		auto tb = static_cast<Term*>(b);

		assert(ta->name == tb->name);
		assert(ta->args.size() == tb->args.size());

		for (int i { 0 }; i != ta->args.size(); ++i)
			unify(ta->args[i], tb->args[i]);
	}
}

}
