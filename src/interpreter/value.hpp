#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "value_tag.hpp"
#include "gc_cell.hpp"

namespace AST {
struct FunctionLiteral;
}

namespace Interpreter {

struct Interpreter;
struct Reference;
struct Value;

using Identifier = InternedString;
using StringType = std::string;
using RecordType = std::unordered_map<Identifier, Value>;
using ArrayType = std::vector<Reference*>;
using FunctionType = AST::FunctionLiteral*;
using NativeFunctionType = auto(Span<Value>, Interpreter&) -> Value;
using CapturesType = std::vector<Reference*>;

inline bool is_heap_type(ValueTag tag) {
	return tag != ValueTag::Null && tag != ValueTag::Boolean && tag != ValueTag::Integer && tag != ValueTag::Float;
}

struct Value {
	explicit Value(GcCell* ptr)
	    : tag {ptr ? ptr->type() : ValueTag::Null}
	    , ptr {ptr} {}

	explicit Value(std::nullptr_t)
	    : tag {ValueTag::Null}
	    , ptr {nullptr} {}

	explicit Value(bool boolean)
	    : tag {ValueTag::Boolean}
	    , as_boolean {boolean} {}

	explicit Value(int integer)
	    : tag {ValueTag::Integer}
	    , as_integer {integer} {}

	explicit Value(float number)
	    : tag {ValueTag::Float}
	    , as_float {number} {}

	Value()
	    : tag {ValueTag::Null}
	    , ptr {nullptr} {}

	GcCell& operator*() {
		assert(is_heap_type(tag));
		return *ptr;
	};

	GcCell* get() {
		assert(is_heap_type(tag));
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
	GcCell* ptr;
	bool as_boolean;
	int as_integer;
	float as_float;
	};
};

void print(Value v, int d = 0);

struct String : GcCell {
	std::string m_value = "";

	String();
	String(std::string s);
};

struct Array : GcCell {
	ArrayType m_value;

	Array();
	Array(ArrayType l);

	void append(Reference* v);
	Reference* at(int position);
};

struct Record : GcCell {
	RecordType m_value;

	Record();
	Record(RecordType);

	void addMember(Identifier const& id, Value v);
	Value getMember(Identifier const& id);
};

struct Variant : GcCell {
	InternedString m_constructor;
	Value m_inner_value {nullptr}; // empty constructor

	Variant(InternedString constructor);
	Variant(InternedString constructor, Value v);
};

struct Function : GcCell {
	FunctionType m_def;
	CapturesType m_captures;

	Function(FunctionType, CapturesType);
};

struct NativeFunction : GcCell {
	NativeFunctionType* m_fptr;

	NativeFunction(NativeFunctionType* = nullptr);
};

struct Reference : GcCell {
	Value m_value;

	Reference(Value value);
};

struct VariantConstructor : GcCell {
	InternedString m_constructor;

	VariantConstructor(InternedString constructor);
};

struct RecordConstructor : GcCell {
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
inline T* Value::get_cast() {
	static_assert(std::is_base_of<GcCell, T>::value, "T is not a subclass of GcCell");
	assert(is_heap_type(tag));
	assert(tag == type_data<T>::tag);
	assert(ptr);
	return static_cast<T*>(ptr);
}

} // namespace Interpreter
