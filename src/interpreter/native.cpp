#include "environment.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "value_type.hpp"

#include <iostream>
#include <sstream>

#include <cassert>

namespace Interpreter {

// TODO: All of these should return gc_ptr

// print(...) prints the values or references in ...
Value* print(ArrayType v, Environment& e) {
	for (auto value : v) {
		print(value);
	}
	return e.null();
}

// array_append(arr, ...) appends the values or references
// in ... to the array
Value* array_append(ArrayType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() > 0);
	assert(unboxed(v[0])->type() == value_type::Array);
	Array* array = static_cast<Array*>(unboxed(v[0]));
	for (unsigned int i = 1; i < v.size(); i++) {
		array->m_value.push_back(unboxed(v[i]));
	}
	return array;
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Value* array_extend(ArrayType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	assert(unboxed(v[0])->type() == value_type::Array);
	assert(unboxed(v[1])->type() == value_type::Array);
	Array* arr1 = static_cast<Array*>(unboxed(v[0]));
	Array* arr2 = static_cast<Array*>(unboxed(v[1]));
	arr1->m_value.insert(
	    arr1->m_value.end(), arr2->m_value.begin(), arr2->m_value.end());
	return arr1;
}

// size(array) returns the size of the array
Value* size(ArrayType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 1);
	assert(unboxed(v[0])->type() == value_type::Array);
	Array* array = static_cast<Array*>(unboxed(v[0]));

	// TODO: don't get()
	return e.new_integer(array->m_value.size()).get();
}

// array_join(array, string) returns a string with
// the array values separated by the string element
Value* array_join(ArrayType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	assert(unboxed(v[0])->type() == value_type::Array);
	assert(unboxed(v[1])->type() == value_type::String);
	Array* array = static_cast<Array*>(unboxed(v[0]));
	String* string = static_cast<String*>(unboxed(v[1]));
	std::stringstream result;
	for (unsigned int i = 0; i < array->m_value.size(); i++) {
		// TODO make it more general
		auto* value = unboxed(array->m_value[i]);

		assert(value->type() == value_type::Integer);

		result << static_cast<Integer*>(value)->m_value;
		if (i < array->m_value.size() - 1)
			result << string->m_value;
	}
	return e.new_string(result.str()).get();
}

// array_at(array, int i) returns the i-th element of the given array
Value* array_at(ArrayType v, Environment& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	assert(unboxed(v[0])->type() == value_type::Array);
	assert(unboxed(v[1])->type() == value_type::Integer);
	Array* array = static_cast<Array*>(unboxed(v[0]));
	Integer* index = static_cast<Integer*>(unboxed(v[1]));
	assert(index->m_value >= 0);
	assert(index->m_value <  array->m_value.size());
	return array->m_value[index->m_value];
}

Value* dummy(ArrayType v, Environment& e) {
	return e.null();
}

Value* value_add(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		//TODO: don't get()
		return e
		    .new_integer(
		        static_cast<Integer*>(lhs_val)->m_value +
		        static_cast<Integer*>(rhs_val)->m_value)
		    .get();
	case value_type::Float:
		return e
		    .new_float(
		        static_cast<Float*>(lhs_val)->m_value +
		        static_cast<Float*>(rhs_val)->m_value)
		    .get();
	case value_type::String:
		return e
		    .new_string(
		        static_cast<String*>(lhs_val)->m_value +
		        static_cast<String*>(rhs_val)->m_value)
		    .get();
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_sub(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		// TODO: don't get()
		return e
		    .new_integer(
		        static_cast<Integer*>(lhs_val)->m_value -
		        static_cast<Integer*>(rhs_val)->m_value)
		    .get();
	case value_type::Float:
		return e
		    .new_float(
		        static_cast<Float*>(lhs_val)->m_value -
		        static_cast<Float*>(rhs_val)->m_value)
		    .get();
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_mul(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		// TODO: don't get()
		return e
		    .new_integer(
		        static_cast<Integer*>(lhs_val)->m_value *
		        static_cast<Integer*>(rhs_val)->m_value)
		    .get();
	case value_type::Float:
		return e
		    .new_float(
		        static_cast<Float*>(lhs_val)->m_value *
		        static_cast<Float*>(rhs_val)->m_value)
		    .get();
	default:
		std::cerr << "ERROR: can't multiply values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_div(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		// TODO: don't get()
		return e
		    .new_integer(
		        static_cast<Integer*>(lhs_val)->m_value /
		        static_cast<Integer*>(rhs_val)->m_value)
		    .get();
	case value_type::Float:
		return e
		    .new_float(
		        static_cast<Float*>(lhs_val)->m_value /
		        static_cast<Float*>(rhs_val)->m_value)
		    .get();
	default:
		std::cerr << "ERROR: can't divide values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_logicand(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == value_type::Boolean and
	    rhs_val->type() == value_type::Boolean)
		return e
		    .new_boolean(
		        static_cast<Boolean*>(lhs_val)->m_value and
		        static_cast<Boolean*>(rhs_val)->m_value)
		    .get();
	std::cerr << "ERROR: logical and operator not defined for types "
	          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_type_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Value* value_logicor(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == value_type::Boolean and
	    rhs_val->type() == value_type::Boolean)
		return e
		    .new_boolean(
		        static_cast<Boolean*>(lhs_val)->m_value or
		        static_cast<Boolean*>(rhs_val)->m_value)
		    .get();
	std::cerr << "ERROR: logical or operator not defined for types "
	          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_type_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Value* value_logicxor(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == value_type::Boolean and
	    rhs_val->type() == value_type::Boolean)
		return e
		    .new_boolean(
		        static_cast<Boolean*>(lhs_val)->m_value !=
		        static_cast<Boolean*>(rhs_val)->m_value)
		    .get();
	std::cerr << "ERROR: exclusive or operator not defined for types "
	          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_type_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Value* value_equals(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());

	switch (lhs_val->type()) {
	case value_type::Null:
		return e.new_boolean(true).get();
	case value_type::Integer:
		return e
		    .new_boolean(
		        static_cast<Integer*>(lhs_val)->m_value ==
		        static_cast<Integer*>(rhs_val)->m_value)
		    .get();
	case value_type::Float:
		return e
		    .new_boolean(
		        static_cast<Float*>(lhs_val)->m_value ==
		        static_cast<Float*>(rhs_val)->m_value)
		    .get();
	case value_type::String:
		return e
		    .new_boolean(
		        static_cast<String*>(lhs_val)->m_value ==
		        static_cast<String*>(rhs_val)->m_value)
		    .get();
	case value_type::Boolean:
		return e
		    .new_boolean(
		        static_cast<Boolean*>(lhs_val)->m_value ==
		        static_cast<Boolean*>(rhs_val)->m_value)
		    .get();
	default: {
		std::cerr << "ERROR: can't compare equality of types "
		          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
		          << value_type_string[static_cast<int>(rhs_val->type())];
		assert(0);
	}
	}
}

Value* value_less(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());

	switch (lhs_val->type()) {
	case value_type::Integer:
		return e
		    .new_boolean(
		        static_cast<Integer*>(lhs_val)->m_value <
		        static_cast<Integer*>(rhs_val)->m_value)
		    .get();
	case value_type::Float:
		return e
		    .new_boolean(
		        static_cast<Float*>(lhs_val)->m_value <
		        static_cast<Float*>(rhs_val)->m_value)
		    .get();
	case value_type::String:
		return e
		    .new_boolean(
		        static_cast<String*>(lhs_val)->m_value <
		        static_cast<String*>(rhs_val)->m_value)
		    .get();
	default:
		std::cerr << "ERROR: can't compare values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Value* value_assign(ArrayType v, Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	// TODO: proper error handling
	assert(lhs->type() == value_type::Reference);
	// NOTE: copied by reference, matters if rhs is actually a reference
	// TODO: change in another pr, perhaps adding Environment::copy_value?
	static_cast<Reference*>(lhs)->m_value = rhs_val;
	return e.null();
}

void declare_native_functions(Environment& env) {
	env.declare(
	    "print", env.new_native_function(static_cast<NativeFunctionType*>(&print)));

	env.declare(
	    "array_append",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_append)));

	env.declare(
	    "array_extend",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_extend)));

	env.declare(
	    "size", env.new_native_function(static_cast<NativeFunctionType*>(&size)));

	env.declare(
	    "array_join",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_join)));

	env.declare(
	    "array_at",
	    env.new_native_function(static_cast<NativeFunctionType*>(&array_at)));

	env.declare(
	    "+", env.new_native_function(static_cast<NativeFunctionType*>(&value_add)));
	env.declare(
	    "-", env.new_native_function(static_cast<NativeFunctionType*>(&value_sub)));
	env.declare(
	    "*", env.new_native_function(static_cast<NativeFunctionType*>(&value_mul)));
	env.declare(
	    "/", env.new_native_function(static_cast<NativeFunctionType*>(&value_div)));
	env.declare(
	    "<",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_less)));
	env.declare(
	    "=",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_assign)));
	env.declare(
	    "==",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_equals)));
	env.declare(
	    "^^",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_logicxor)));
	env.declare(
	    "&&",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_logicand)));
	env.declare(
	    "||",
	    env.new_native_function(static_cast<NativeFunctionType*>(&value_logicor)));
}

} // namespace Interpreter
