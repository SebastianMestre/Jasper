#include <assert>
#include <string>
#include <vector>
#include <unordered_map>
#include "GarbageCollector.hpp"

/* types */
using Identifier = std::string;
using ObjectType = unordered_map<Identifier, Value>;
using ListType = vector<Value>;
// using FunctionType = <syntax tree>

struct Value {
	Identifier identifier;
	int refcount;

	Value(Identifier id) {
		identifier = id;
	}

	~Value() {
		GarbageCollector::soft_clean(this);
	}

	/* something has a new reference of this */
	void added_reference() {
		refcount++;
	}

	/* something lost a reference to this */
	void lost_reference() {
		assert(refcount > 0);
		refcount--;
		if(refcount > 0)
			GarbageCollector::soft_clean(this);
	}

	virtual vector<Value*> references() = 0;
};

struct Integer : Value {
	int value;

	Integer(Identifier id, int v) : Value(id) {
		value = v;
	}
}

struct String : Value {
	std::string value;

	String(Identifier id, std::string s) : Value(id) {
		value = s;
	}
}

struct List : Value {
	ListType value;

	List(Identifier id, ListType& l = {}) : Value(id) {
		value = l; // no se hacer esto. (std::move?)
	}
}

struct Object : Value {
	ObjectType value;

	List(Identifier id, ObjectType& l = {}) : Value(id) {
		value = l; // no se hacer esto. (std::move?)
	}
}

//struct Function : Value {
//	FunctionType value;
//
//	Function(Identifier id, FunctionType& f = {}) : Value(id) {
//		value = f; // no se hacer esto. (std::move?)
//	}
//}
/**
 * Example:
 *
 * Types::Integer create_int(Identifier id, int value) {
 * 	return Types::Integer(id, value);
 * }
 * 
 * Types::List create_list(Identifier id, vector<Value> &vv) {
 * 	return Types::List(id, vv);
 * }
 */
