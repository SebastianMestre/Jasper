#pragma once

#include <assert>
#include <string>
#include <vector>
#include <unordered_map>
#include "GarbageCollector.hpp"

using namespace Type;

/* types */
using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = vector<Value*>;
// using FunctionType = <syntax tree>

struct Value {
	int refcount;

	/* something has a new reference of this */
	void added_reference() {
		refcount++;
	}

	/* something lost a reference to this */
	void lost_reference() {
		assert(refcount > 0);
		refcount--;
	}
};

struct Integer : Value {
	int value;

	Integer(int v) : value(v) {}
}

struct String : Value {
	std::string value;

	String(std::string s = "") : value(s) {}
}

struct List : Value {
	ListType* value;

	List(ListType* l = new ListType) : value(l) {}

	append(Value* v) {
		value->push_back(v);
	}

	at(int position) {
		if (position < 0 or position >= value->size()) {
			// return OutOfBounds
		} else {
			return *(value)[position];
		}
	}
}

struct Object : Value {
	ObjectType* value;

	Object(ObjectType* o = new ObjectType) : value(o) {}

	addMember(Identifier id, Value* v) {
		*(value)[id] = v;
	}

	getMember(Identifier id) {
		if( value->find(id) == value->end() ) {
			// return ReferenceError
		} else {
			return *(value)[id];
		}
	}
}

//struct Function : Value {
//	FunctionType* value;
//
//	Function(FunctionType* f) : value(f) {
//}

/**
 * Example:
 *
 * using namespace Type;
 *
 * TopLevel = new Object;
 * TopLevel.addMember(the_new_id, new Integer(10));
 * TopLevel.addMember(
 * 	the_other_new_id,
 * 	new List( {new Integer(2), new Integer(2)} )
 * );
 * 
 */
