#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

namespace Type {

struct Value;

/* types */
using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = std::vector<Value*>;
// using FunctionType = <syntax tree>

struct Value {
	bool visited = false;
	virtual void gc_visit() = 0;
	virtual ~Value() = default;
};

struct Integer : Value {
	int value = 0;

	Integer() = default;
	Integer(int v);

	void gc_visit() override;
};

struct String : Value {
	std::string value = "";

	String() = default;
	String(std::string s);

	void gc_visit() override;
};

struct List : Value {
	ListType value;

	List() = default;
	List(ListType l);

	void append(Value* v);
	Value* at(int position);

	void gc_visit() override;
};

struct Object : Value {
	ObjectType value;

	Object() = default;
	Object(ObjectType);

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);

	void gc_visit() override;
};

// TODO: define data representation of functions

/**
 * Example:
 *
 * using namespace Type;
 * using GarbageCollector::GC;
 *
 * GC gc;
 *
 * Object* top_level = gc.new_object();
 * gc.add_root(top_level);
 *
 * TopLevel.add_member("myInt", gc.new_integer(10));
 *
 * TopLevel.add_member(
 *     "myList",
 *     gc.new_list({
 *         gc.new_integer(2),
 *         gc.new_integer(2)
 * }));
 *
 */

} // namespace Type
