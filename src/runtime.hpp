#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.hpp"

namespace Type {

struct Value {
	bool m_visited = false;
	virtual void gc_visit() = 0;
	virtual ~Value() = default;
};

struct Null : Value {

	Null() = default;

	void gc_visit() override;
};

struct Integer : Value {
	int m_value = 0;

	Integer() = default;
	Integer(int v);

	void gc_visit() override;
};

struct String : Value {
	std::string m_value = "";

	String() = default;
	String(std::string s);

	void gc_visit() override;
};

struct List : Value {
	ListType m_value;

	List() = default;
	List(ListType l);

	void append(Value* v);
	Value* at(int position);

	void gc_visit() override;
};

struct Object : Value {
	ObjectType m_value;

	Object() = default;
	Object(ObjectType);

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);

	void gc_visit() override;
};

struct Function : Value {
	FunctionType m_def;
	Environment m_env;

	Function() = default;
	Function(FunctionType, Environment);
	
	Value* call(ListType args);

	void gc_visit() override;
};


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
