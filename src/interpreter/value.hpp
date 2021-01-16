#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "value_fwd.hpp"
#include "value_tag.hpp"

namespace TypedAST {
struct FunctionLiteral;
}

namespace Interpreter {

struct Interpreter;

struct Reference;

using Identifier = InternedString;
using StringType = std::string;
using RecordType = std::unordered_map<Identifier, Value*>;
using DictionaryType = std::unordered_map<StringType, Value*>;
using ArrayType = std::vector<Reference*>;
using FunctionType = TypedAST::FunctionLiteral*;
using NativeFunctionType = auto(Span<Value*>, Interpreter&) -> Value*;
using CapturesType = std::vector<std::pair<Identifier, Value*>>; // TODO: store references instead of values

// Returns the value pointed to by a reference
void print(Value* v, int d = 0);
void gc_visit(Value*);

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

	void addMember(Identifier const& id, Value* v);
	Value* getMember(Identifier const& id);
};

struct Dictionary : Value {
	DictionaryType m_value;

	Dictionary();
	Dictionary(DictionaryType);

	void addMember(StringType const& id, Value* v);
	Value* getMember(StringType const& id);
	void removeMember(StringType const& id);
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
	Value* m_value;

	Reference(Value* value);
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
