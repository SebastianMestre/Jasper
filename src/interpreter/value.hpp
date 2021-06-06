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
struct Handle;

using Identifier = InternedString;
using StringType = std::string;
using RecordType = std::unordered_map<Identifier, Handle>;
using ArrayType = std::vector<Reference*>;
using FunctionType = AST::FunctionLiteral*;
using NativeFunctionType = auto(Span<Handle>, Interpreter&) -> Handle;
using CapturesType = std::vector<Reference*>;

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

inline bool is_heap_type(ValueTag tag) {
	return tag != ValueTag::Null && tag != ValueTag::Boolean && tag != ValueTag::Integer && tag != ValueTag::Float;
}

struct Handle {
	Handle(Value* ptr)
	    : tag {ptr ? ptr->type() : ValueTag::Null}
	    , ptr {ptr} {}

	Handle(std::nullptr_t)
	    : tag {ValueTag::Null}
	    , ptr {nullptr} {}

	Handle(bool boolean)
	    : tag {ValueTag::Boolean}
	    , as_boolean {boolean} {}

	Handle(int integer)
	    : tag {ValueTag::Integer}
	    , as_integer {integer} {}

	Handle(float number)
	    : tag {ValueTag::Float}
	    , as_float {number} {}

	Handle()
	    : tag {ValueTag::Null}
	    , ptr {nullptr} {}

	Value& operator*() {
		return *ptr;
	};

	Value* get() {
		return ptr;
	}

	template <typename T>
	T* get_cast();

	int get_integer() {
		assert(tag == ValueTag::Integer);
		return as_integer;
	}

	float get_float() {
		assert(tag == ValueTag::Float);
		return as_float;
	}

	bool get_boolean() {
		assert(tag == ValueTag::Boolean);
		return as_boolean;
	}

	ValueTag type() {
		if (is_heap_type(tag)) {
			assert(ptr);
			assert(ptr->type() == tag);
		}
		return tag;
	}

	ValueTag tag;
	union {
	Value* ptr;
	bool as_boolean;
	int as_integer;
	float as_float;
	};
};

void gc_visit(Handle);
void print(Handle v, int d = 0);

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
	Reference* at(int position);
};

struct Record : Value {
	RecordType m_value;

	Record();
	Record(RecordType);

	void addMember(Identifier const& id, Handle v);
	Handle getMember(Identifier const& id);
};

struct Variant : Value {
	InternedString m_constructor;
	Handle m_inner_value {nullptr}; // empty constructor

	Variant(InternedString constructor);
	Variant(InternedString constructor, Handle v);
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

template<typename T>
struct type_data;

template<> struct type_data<String> { static constexpr auto tag = ValueTag::String; };
template<> struct type_data<Array> { static constexpr auto tag = ValueTag::Array; };
template<> struct type_data<Record> { static constexpr auto tag = ValueTag::Record; };
template<> struct type_data<Variant> { static constexpr auto tag = ValueTag::Variant; };
template<> struct type_data<Function> { static constexpr auto tag = ValueTag::Function; };
template<> struct type_data<NativeFunction> { static constexpr auto tag = ValueTag::NativeFunction; };
template<> struct type_data<Reference> { static constexpr auto tag = ValueTag::Reference; };
template<> struct type_data<VariantConstructor> { static constexpr auto tag = ValueTag::VariantConstructor; };
template<> struct type_data<RecordConstructor> { static constexpr auto tag = ValueTag::RecordConstructor; };

template <typename T>
inline T* Handle::get_cast() {
	static_assert(std::is_base_of<Value, T>::value, "T is not a subclass of Value");
	assert(is_heap_type(tag));
	assert(tag == type_data<T>::tag);
	assert(ptr);
	return static_cast<T*>(ptr);
}

} // namespace Interpreter
