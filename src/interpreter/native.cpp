#include "../utils/span.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "value_tag.hpp"

#include <iostream>
#include <sstream>

#include <cassert>

namespace Interpreter {

// TODO: All of these should return gc_ptr

using ArgsType = Span<Value*>;

// print(vals...) prints the values or references in vals
Value* print(ArgsType v, Interpreter& e) {
	for (auto value : v)
		print(value);
	return e.null();
}

// array_append(arr, vals...) appends the values in vals to the array
Value* array_append(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() > 0);
	Array* array = value_as<Array>(v[0]);
	for (unsigned int i = 1; i < v.size(); i++) {
		array->append(e.new_reference(value_of(v[i])).get());
	}
	return array;
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Value* array_extend(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	Array* arr1 = value_as<Array>(v[0]);
	Array* arr2 = value_as<Array>(v[1]);
	arr1->m_value.insert(
	    arr1->m_value.end(), arr2->m_value.begin(), arr2->m_value.end());
	return arr1;
}

// size(array) returns the size of the array
Value* size(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() == 1);
	Array* array = value_as<Array>(v[0]);

	return e.m_gc->new_integer_raw(array->m_value.size());
}

// array_join(array, string) returns a string with
// the array values separated by the string element
Value* array_join(ArgsType v, Interpreter& e) {
	// TODO make it more general
	// TODO proper error handling
	assert(v.size() == 2);
	Array* array = value_as<Array>(v[0]);
	String* sep = value_as<String>(v[1]);
	std::stringstream result;
	for (unsigned int i = 0; i < array->m_value.size(); i++) {
		if (i > 0) result << sep->m_value;
		result << value_as<Integer>(array->m_value[i])->m_value;
	}
	return e.m_gc->new_string_raw(result.str());
}

// array_at(array, int i) returns the i-th element of the given array
Value* array_at(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	Array* array = value_as<Array>(v[0]);
	Integer* index = value_as<Integer>(v[1]);
	assert(index->m_value >= 0);
	assert(index->m_value < array->m_value.size());
	return array->m_value[index->m_value];
}

Value* dummy(ArgsType v, Interpreter& e) {
	return e.null();
}

Value* value_add(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());
	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs)->m_value +
		    static_cast<Integer*>(rhs)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs)->m_value +
		    static_cast<Float*>(rhs)->m_value);
	case ValueTag::String:
		return e.m_gc->new_string_raw(
		    static_cast<String*>(lhs)->m_value +
		    static_cast<String*>(rhs)->m_value);
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_string[static_cast<int>(lhs->type())];
		assert(0);
	}
}

Value* value_sub(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());
	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs)->m_value -
		    static_cast<Integer*>(rhs)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs)->m_value -
		    static_cast<Float*>(rhs)->m_value);
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_string[static_cast<int>(lhs->type())];
		assert(0);
	}
}

Value* value_mul(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());
	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs)->m_value *
		    static_cast<Integer*>(rhs)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs)->m_value *
		    static_cast<Float*>(rhs)->m_value);
	default:
		std::cerr << "ERROR: can't multiply values of type "
		          << value_string[static_cast<int>(lhs->type())];
		assert(0);
	}
}

Value* value_div(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());
	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_integer_raw(
		    static_cast<Integer*>(lhs)->m_value /
		    static_cast<Integer*>(rhs)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_float_raw(
		    static_cast<Float*>(lhs)->m_value /
		    static_cast<Float*>(rhs)->m_value);
	default:
		std::cerr << "ERROR: can't divide values of type "
		          << value_string[static_cast<int>(lhs->type())];
		assert(0);
	}
}

Value* value_logicand(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	if (lhs->type() == ValueTag::Boolean and rhs->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs)->m_value and
		    static_cast<Boolean*>(rhs)->m_value);
	std::cerr << "ERROR: logical and operator not defined for types "
	          << value_string[static_cast<int>(lhs->type())] << " and "
	          << value_string[static_cast<int>(rhs->type())];
	assert(0);
}

Value* value_logicor(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	if (lhs->type() == ValueTag::Boolean and rhs->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs)->m_value or
		    static_cast<Boolean*>(rhs)->m_value);
	std::cerr << "ERROR: logical or operator not defined for types "
	          << value_string[static_cast<int>(lhs->type())] << " and "
	          << value_string[static_cast<int>(rhs->type())];
	assert(0);
}

Value* value_logicxor(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	if (lhs->type() == ValueTag::Boolean and rhs->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs)->m_value !=
		    static_cast<Boolean*>(rhs)->m_value);
	std::cerr << "ERROR: exclusive or operator not defined for types "
	          << value_string[static_cast<int>(lhs->type())] << " and "
	          << value_string[static_cast<int>(rhs->type())];
	assert(0);
}

Value* value_equals(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());

	switch (lhs->type()) {
	case ValueTag::Null:
		return e.m_gc->new_boolean_raw(true);
	case ValueTag::Integer:
		return e.m_gc->new_boolean_raw(
		    static_cast<Integer*>(lhs)->m_value ==
		    static_cast<Integer*>(rhs)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_boolean_raw(
		    static_cast<Float*>(lhs)->m_value ==
		    static_cast<Float*>(rhs)->m_value);
	case ValueTag::String:
		return e.m_gc->new_boolean_raw(
		    static_cast<String*>(lhs)->m_value ==
		    static_cast<String*>(rhs)->m_value);
	case ValueTag::Boolean:
		return e.m_gc->new_boolean_raw(
		    static_cast<Boolean*>(lhs)->m_value ==
		    static_cast<Boolean*>(rhs)->m_value);
	default: {
		std::cerr << "ERROR: can't compare equality of types "
		          << value_string[static_cast<int>(lhs->type())] << " and "
		          << value_string[static_cast<int>(rhs->type())];
		assert(0);
	}
	}
}

Value* value_less(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());

	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_boolean_raw(
		    static_cast<Integer*>(lhs)->m_value <
		    static_cast<Integer*>(rhs)->m_value);
	case ValueTag::Float:
		return e.m_gc->new_boolean_raw(
		    static_cast<Float*>(lhs)->m_value <
		    static_cast<Float*>(rhs)->m_value);
	case ValueTag::String:
		return e.m_gc->new_boolean_raw(
		    static_cast<String*>(lhs)->m_value <
		    static_cast<String*>(rhs)->m_value);
	default:
		std::cerr << "ERROR: can't compare values of type "
		          << value_string[static_cast<int>(lhs->type())];
		assert(0);
	}
}

Value* value_assign(ArgsType v, Interpreter& e) {
	auto* lhs = v[0];
	auto* rhs = value_of(v[1]);

	// NOTE: copied by reference, matters if rhs is actually a reference
	// TODO: change in another pr, perhaps adding Interpreter::copy_value?
	as<Reference>(lhs)->m_value = rhs;
	return e.null();
}

void declare_native_functions(Interpreter& env) {
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
