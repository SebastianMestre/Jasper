#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

#include "value_type.hpp"

struct ASTFunctionLiteral;

namespace Type {

struct Scope;
struct Value;

using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = std::vector<Value*>;
using FunctionType = ::ASTFunctionLiteral*;



struct Value {
protected:
	value_type m_type;

public:
	bool m_visited = false;

	Value(value_type type) : m_type(type) {}
	value_type type() const { return m_type; }

	virtual void gc_visit() = 0;
	virtual ~Value() = default;
};

struct Null : Value {

	Null();

	void gc_visit() override;
};

struct Integer : Value {
	int m_value = 0;

	Integer();
	Integer(int v);

	void gc_visit() override;
};

struct Float : Value {
	float m_value = 0.0;

	Float();
	Float(float v);

	void gc_visit() override;
};

struct Boolean : Value {
	bool m_value = false;

	Boolean();
	Boolean(bool b);

	void gc_visit() override;
};

struct String : Value {
	std::string m_value = "";

	String();
	String(std::string s);

	void gc_visit() override;
};

struct List : Value {
	ListType m_value;

	List();
	List(ListType l);

	void append(Value* v);
	Value* at(int position);

	void gc_visit() override;
};

struct Object : Value {
	ObjectType m_value;

	Object();
	Object(ObjectType);

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);
	
	void gc_visit() override;
};

struct Dictionary : Value {
	ObjectType m_value;

	Dictionary();
	Dictionary(ObjectType);

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);
	void removeMember(Identifier const& id);

	void gc_visit() override;
};

struct Function : Value {
	FunctionType m_def;
	Scope* m_scope;

	Function();
	Function(FunctionType, Scope*);

	void gc_visit() override;
};

Value* call(Function* f, ListType args);

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
