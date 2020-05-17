#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "value_type.hpp"

struct ASTFunctionLiteral;

namespace Type {

struct Scope;
struct Value;
struct Environment;

using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ArrayType = std::vector<Value*>;
using FunctionType = ::ASTFunctionLiteral*;
using NativeFunctionType = auto(Value*, Environment&) -> Value*;

void print(Value* v, int d = 0);
void gc_visit(Value*);

struct Value {
protected:
	value_type m_type;

public:
	bool m_visited = false;

	Value(value_type type) : m_type(type) {}
	value_type type() const { return m_type; }

	virtual ~Value() = default;
};


struct Null : Value {

	Null();
};

struct Integer : Value {
	int m_value = 0;

	Integer();
	Integer(int v);
};

struct Float : Value {
	float m_value = 0.0;

	Float();
	Float(float v);
};

struct Boolean : Value {
	bool m_value = false;

	Boolean();
	Boolean(bool b);
};

struct String : Value {
	std::string m_value = "";

	String();
	String(std::string s);
};

struct Array : Value {
	ArrayType m_value;

	Array();
	Array(ArrayType l);

	void append(Value* v);
	Value* at(int position);
};

struct Object : Value {
	ObjectType m_value;

	Object();
	Object(ObjectType);

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);
};

struct Dictionary : Value {
	ObjectType m_value;

	Dictionary();
	Dictionary(ObjectType);

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);
	void removeMember(Identifier const& id);
};

struct Function : Value {
	FunctionType m_def;
	// TODO: store references instead of values
	ObjectType m_captures;

	Function(FunctionType, ObjectType);
};

struct NativeFunction : Value {
	NativeFunctionType* m_fptr;

	NativeFunction(NativeFunctionType* = nullptr);
};

struct Reference : Value {
	Value* m_value;

	Reference(Value* value);
};

} // namespace Type
