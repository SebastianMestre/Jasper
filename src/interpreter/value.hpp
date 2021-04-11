#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "value_fwd.hpp"
#include "value_tag.hpp"

namespace AST {
struct FunctionLiteral;
}

namespace Interpreter {

struct Interpreter;

struct Reference;

struct Handle {
	ValueTag m_tag;
	Value* as_value;
	Handle(Handle const&) = default;
	Handle(Handle&&) = default;
	Handle& operator=(Handle const&) = default;
	Handle& operator=(Handle&&) = default;
	Handle(Value* value);
	Value* get() { return as_value; }
};

using Identifier = InternedString;
using StringType = std::string;
using RecordType = std::unordered_map<Identifier, Handle>;
using ArrayType = std::vector<Reference*>;
using FunctionType = AST::FunctionLiteral*;
using NativeFunctionType = auto(Span<Handle>, Interpreter&) -> Handle;
using CapturesType = std::vector<Reference*>;

// Returns the value pointed to by a reference
void print(Value* v, int d = 0);
inline void print(Handle v, int d = 0) {
	return print(v.get(), d);
}

void gc_visit(Value*);
inline void gc_visit(Handle h) {
	return gc_visit(h.get());
}

struct Value {
  protected:
	ValueTag m_type;

  public:
	bool m_visited = false;
	int m_cpp_refcount = 0;

	Value(ValueTag type)
	    : m_type(type) {}
	ValueTag type() const {
		return m_type;
	}

	virtual ~Value() = default;
};

inline Handle::Handle(Value* value)
    : m_tag {value->type()}
    , as_value {value} {}

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

	void append(Reference* v);
	Value* at(int position);
};

struct Record : Value {
	RecordType m_value;

	Record();
	Record(RecordType);

	void set_member(Identifier const& id, Handle v);
	Handle get_member(Identifier const& id);
};

struct Variant : Value {
	InternedString m_constructor;
	Value* m_inner_value {nullptr}; // empty constructor

	Variant(InternedString constructor);
	Variant(InternedString constructor, Value* v);
};

struct Function : Value {
	FunctionType m_def;
	CapturesType m_captures;

	Function(FunctionType, CapturesType);
};

struct NativeFunction : Value {
	NativeFunctionType* m_fptr;

	NativeFunction(NativeFunctionType* = nullptr);
};

struct Reference : Value {
	Handle m_value;

	Reference(Handle value);
};

struct VariantConstructor : Value {
	InternedString m_constructor;

	VariantConstructor(InternedString constructor);
};

struct RecordConstructor : Value {
	std::vector<InternedString> m_keys;

	RecordConstructor(std::vector<InternedString> keys);
};

} // namespace Interpreter
