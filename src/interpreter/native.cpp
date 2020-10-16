#include "../span.hpp"
#include "environment.hpp"
#include "garbage_collector.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "value_tag.hpp"

#include <iostream>
#include <sstream>

#include <cassert>

namespace Interpreter {

// TODO: All of these should return gc_ptr

using ArgsType = Span<Value*>;

// print(...) prints the values or references in ...
Value* print(ArgsType v, Environment& e) {
	for (auto value : v) {
		print(value);
	}
	return e.null();
}

// array_append(arr, ...) appends the values or references
// in ... to the array
Value* array_append(ArgsType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() > 0);
	assert(unboxed(v[0])->type() == ValueTag::Array);
	Array* array = static_cast<Array*>(unboxed(v[0]));
	for (unsigned int i = 1; i < v.size(); i++) {
		array->m_value.push_back(unboxed(v[i]));
	}
	return array;
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Value* array_extend(ArgsType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	assert(unboxed(v[0])->type() == ValueTag::Array);
	assert(unboxed(v[1])->type() == ValueTag::Array);
	Array* arr1 = static_cast<Array*>(unboxed(v[0]));
	Array* arr2 = static_cast<Array*>(unboxed(v[1]));
	arr1->m_value.insert(
	    arr1->m_value.end(), arr2->m_value.begin(), arr2->m_value.end());
	return arr1;
}

// size(array) returns the size of the array
Value* size(ArgsType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 1);
	assert(unboxed(v[0])->type() == ValueTag::Array);
	Array* array = static_cast<Array*>(unboxed(v[0]));

	// TODO: don't get()
	return e.m_gc->new_integer_raw(array->m_value.size());
}

// array_join(array, string) returns a string with
// the array values separated by the string element
Value* array_join(ArgsType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	assert(unboxed(v[0])->type() == ValueTag::Array);
	assert(unboxed(v[1])->type() == ValueTag::String);
	Array* array = static_cast<Array*>(unboxed(v[0]));
	String* string = static_cast<String*>(unboxed(v[1]));
	std::stringstream result;
	for (unsigned int i = 0; i < array->m_value.size(); i++) {
		// TODO make it more general
		auto* value = unboxed(array->m_value[i]);

		assert(value->type() == ValueTag::Integer);

		result << static_cast<Integer*>(value)->m_value;
		if (i < array->m_value.size() - 1)
			result << string->m_value;
	}
	return e.m_gc->new_string_raw(result.str());
}

// array_at(array, int i) returns the i-th element of the given array
Value* array_at(ArgsType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	assert(unboxed(v[0])->type() == ValueTag::Array);
	assert(unboxed(v[1])->type() == ValueTag::Integer);
	Array* array = static_cast<Array*>(unboxed(v[0]));
	Integer* index = static_cast<Integer*>(unboxed(v[1]));
	assert(index->m_value >= 0);
	assert(index->m_value < array->m_value.size());
	return array->m_value[index->m_value];
}

Value* dummy(ArgsType v, Environment& e) {
	return e.null();
}

Value* value_add(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case ValueTag::Integer:
		//TODO: don't get()
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs_val)->m_value +
		    static_cast<Integer*>(rhs_val)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs_val)->m_value +
		    static_cast<Float*>(rhs_val)->m_value);
	case ValueTag::String:
		return e.m_gc->new_string_raw(
		    static_cast<String*>(lhs_val)->m_value +
		    static_cast<String*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_sub(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case ValueTag::Integer:
		// TODO: don't get()
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs_val)->m_value -
		    static_cast<Integer*>(rhs_val)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs_val)->m_value -
		    static_cast<Float*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_mul(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case ValueTag::Integer:
		// TODO: don't get()
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs_val)->m_value *
		    static_cast<Integer*>(rhs_val)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs_val)->m_value *
		    static_cast<Float*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't multiply values of type "
		          << value_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_div(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case ValueTag::Integer:
		// TODO: don't get()
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs_val)->m_value /
		    static_cast<Integer*>(rhs_val)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs_val)->m_value /
		    static_cast<Float*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't divide values of type "
		          << value_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_logicand(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == ValueTag::Boolean and rhs_val->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs_val)->m_value and
		    static_cast<Boolean*>(rhs_val)->m_value);
	std::cerr << "ERROR: logical and operator not defined for types "
	          << value_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Value* value_logicor(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == ValueTag::Boolean and rhs_val->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs_val)->m_value or
		    static_cast<Boolean*>(rhs_val)->m_value);
	std::cerr << "ERROR: logical or operator not defined for types "
	          << value_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Value* value_logicxor(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == ValueTag::Boolean and rhs_val->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs_val)->m_value !=
		    static_cast<Boolean*>(rhs_val)->m_value);
	std::cerr << "ERROR: exclusive or operator not defined for types "
	          << value_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Value* value_equals(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());

	switch (lhs_val->type()) {
	case ValueTag::Null:
		return e.m_gc->new_boolean_raw(true);
	case ValueTag::Integer:
		return e.m_gc->new_boolean_raw(
		    static_cast<Integer*>(lhs_val)->m_value ==
		    static_cast<Integer*>(rhs_val)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_boolean_raw(
		    static_cast<Float*>(lhs_val)->m_value ==
		    static_cast<Float*>(rhs_val)->m_value);
	case ValueTag::String:
		return e.m_gc->new_boolean_raw(
		    static_cast<String*>(lhs_val)->m_value ==
		    static_cast<String*>(rhs_val)->m_value);
	case ValueTag::Boolean:
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs_val)->m_value ==
		    static_cast<Boolean*>(rhs_val)->m_value);
	default: {
		std::cerr << "ERROR: can't compare equality of types "
		          << value_string[static_cast<int>(lhs_val->type())] << " and "
		          << value_string[static_cast<int>(rhs_val->type())];
		assert(0);
	}
	}
}

Value* value_less(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());

	switch (lhs_val->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_boolean_raw(
		    static_cast<Integer*>(lhs_val)->m_value <
		    static_cast<Integer*>(rhs_val)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_boolean_raw(
		    static_cast<Float*>(lhs_val)->m_value <
		    static_cast<Float*>(rhs_val)->m_value);
	case ValueTag::String:
		return e.m_gc->new_boolean_raw(
		    static_cast<String*>(lhs_val)->m_value <
		    static_cast<String*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't compare values of type "
		          << value_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_assign(ArgsType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	// TODO: proper error handling
	assert(lhs->type() == ValueTag::Reference);
	// NOTE: copied by reference, matters if rhs is actually a reference
	// TODO: change in another pr, perhaps adding Environment::copy_value?
	static_cast<Reference*>(lhs)->m_value = rhs_val;
	return e.null();
}

void declare_native_functions(Environment& env) {
	env.global_declare(
	    "print", env.new_native_function(static_cast<NativeFunctionType*>(&print)));

	env.global_declare(
	    "array_append",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_append)));

	env.global_declare(
	    "array_extend",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_extend)));

	env.global_declare(
	    "size", env.new_native_function(static_cast<NativeFunctionType*>(&size)));

	env.global_declare(
	    "array_join",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_join)));

	env.global_declare(
	    "array_at",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_at)));

	env.global_declare(
	    "+", env.new_native_function(static_cast<NativeFunctionType*>(&value_add)));
	env.global_declare(
	    "-", env.new_native_function(static_cast<NativeFunctionType*>(&value_sub)));
	env.global_declare(
	    "*", env.new_native_function(static_cast<NativeFunctionType*>(&value_mul)));
	env.global_declare(
	    "/", env.new_native_function(static_cast<NativeFunctionType*>(&value_div)));
	env.global_declare(
	    "<",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_less)));
	env.global_declare(
	    "=",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_assign)));
	env.global_declare(
	    "==",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_equals)));
	env.global_declare(
	    "^^",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_logicxor)));
	env.global_declare(
	    "&&",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_logicand)));
	env.global_declare(
	    "||",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_logicor)));
}

} // namespace Interpreter
