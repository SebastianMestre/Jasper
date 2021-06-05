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

#define OP(type, lhs, op, rhs) \
	static_cast<type*>(lhs)->m_value op static_cast<type*>(rhs)->m_value

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

Value* value_add(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());
	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_integer_raw(OP(Integer, lhs, +, rhs));
	case ValueTag::Float:
		return e.m_gc->new_float_raw(OP(Float, lhs, +, rhs));
	case ValueTag::String:
		return e.m_gc->new_string_raw(OP(String, lhs, +, rhs));
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
		return e.m_gc->new_integer_raw(OP(Integer, lhs, -, rhs));
	case ValueTag::Float:
		return e.m_gc->new_float_raw(OP(Float, lhs, -, rhs));
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
		return e.m_gc->new_integer_raw(OP(Integer, lhs, *, rhs));
	case ValueTag::Float:
		return e.m_gc->new_float_raw(OP(Float, lhs, *, rhs));
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
		return e.m_gc->new_integer_raw(OP(Integer, lhs, /, rhs));
	case ValueTag::Float:
		return e.m_gc->new_float_raw(OP(Float, lhs, /, rhs));
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
		return e.m_gc->new_boolean_raw(OP(Boolean, lhs, &&, rhs));
	std::cerr << "ERROR: logical and operator not defined for types "
	          << value_string[static_cast<int>(lhs->type())] << " and "
	          << value_string[static_cast<int>(rhs->type())];
	assert(0);
}

Value* value_logicor(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	if (lhs->type() == ValueTag::Boolean and rhs->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(OP(Boolean, lhs, ||, rhs));
	std::cerr << "ERROR: logical or operator not defined for types "
	          << value_string[static_cast<int>(lhs->type())] << " and "
	          << value_string[static_cast<int>(rhs->type())];
	assert(0);
}

Value* value_logicxor(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	if (lhs->type() == ValueTag::Boolean and rhs->type() == ValueTag::Boolean)
		return e.m_gc->new_boolean_raw(OP(Boolean, lhs, !=, rhs));
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
		return e.m_gc->new_boolean_raw(OP(Integer, lhs, ==, rhs));
	case ValueTag::Float:
		return e.m_gc->new_boolean_raw(OP(Float, lhs, ==, rhs));
	case ValueTag::String:
		return e.m_gc->new_boolean_raw(OP(String, lhs, ==, rhs));
	case ValueTag::Boolean:
		return e.m_gc->new_boolean_raw(OP(Boolean, lhs, ==, rhs));
	default: {
		std::cerr << "ERROR: can't compare equality of types "
		          << value_string[static_cast<int>(lhs->type())] << " and "
		          << value_string[static_cast<int>(rhs->type())];
		assert(0);
	}
	}
}

Value* value_not_equals(ArgsType v, Interpreter& e) {
	Boolean* b = static_cast<Boolean*>(value_equals(v, e));
	b->m_value = !b->m_value;
	return b;
}

Value* value_less(ArgsType v, Interpreter& e) {
	auto* lhs = value_of(v[0]);
	auto* rhs = value_of(v[1]);

	assert(lhs->type() == rhs->type());

	switch (lhs->type()) {
	case ValueTag::Integer:
		return e.m_gc->new_boolean_raw(OP(Integer, lhs, <, rhs));
	case ValueTag::Float:
		return e.m_gc->new_boolean_raw(OP(Float, lhs, <, rhs));
	case ValueTag::String:
		return e.m_gc->new_boolean_raw(OP(String, lhs, <, rhs));
	default:
		std::cerr << "ERROR: can't compare values of type "
		          << value_string[static_cast<int>(lhs->type())];
		assert(0);
	}
}

Value* value_greater_or_equal(ArgsType v, Interpreter& e) {
	Boolean* b = static_cast<Boolean*>(value_less(v, e));
	b->m_value = !b->m_value;
	return b;
}

Value* value_greater(ArgsType v, Interpreter& e) {
	std::swap(v[0], v[1]);
	return value_less(v, e);
}

Value* value_less_or_equal(ArgsType v, Interpreter& e) {
	Boolean* b = static_cast<Boolean*>(value_greater(v, e));
	b->m_value = !b->m_value;
	return b;
}

Value* value_assign(ArgsType v, Interpreter& e) {
	e.assign(v[0], v[1]);
	return e.null();
}

Value* read_integer(ArgsType v, Interpreter& e) {
	// TODO: error handling
	int result;
	std::cin >> result;
	return e.m_gc->new_integer_raw(result);
}

Value* read_number(ArgsType v, Interpreter& e) {
	// TODO: error handling
	float result;
	std::cin >> result;
	return e.m_gc->new_float_raw(result);
}

Value* read_line(ArgsType v, Interpreter& e) {
	// TODO: error handling
	std::string result;
	std::getline(std::cin, result);
	return e.m_gc->new_string_raw(std::move(result));
}

Value* read_string(ArgsType v, Interpreter& e) {
	// TODO: error handling
	std::string result;
	std::cin >> result;
	return e.m_gc->new_string_raw(std::move(result));
}

void declare_native_functions(Interpreter& env) {
	env.global_declare("print", env.new_native_function(print).get());
	env.global_declare("array_append", env.new_native_function(array_append).get());
	env.global_declare("array_extend", env.new_native_function(array_extend).get());
	env.global_declare("size", env.new_native_function(size).get());
	env.global_declare("array_join", env.new_native_function(array_join).get());
	env.global_declare("array_at", env.new_native_function(array_at).get());
	env.global_declare("+", env.new_native_function(value_add).get());
	env.global_declare("-", env.new_native_function(value_sub).get());
	env.global_declare("*", env.new_native_function(value_mul).get());
	env.global_declare("/", env.new_native_function(value_div).get());
	env.global_declare("<", env.new_native_function(value_less).get());
	env.global_declare(">=", env.new_native_function(value_greater_or_equal).get());
	env.global_declare(">", env.new_native_function(value_greater).get());
	env.global_declare("<=", env.new_native_function(value_less_or_equal).get());
	env.global_declare("=", env.new_native_function(value_assign).get());
	env.global_declare("==", env.new_native_function(value_equals).get());
	env.global_declare("!=", env.new_native_function(value_not_equals).get());
	env.global_declare("^^", env.new_native_function(value_logicxor).get());
	env.global_declare("&&", env.new_native_function(value_logicand).get());
	env.global_declare("||", env.new_native_function(value_logicor).get());

	// Input
	env.global_declare("read_integer", env.new_native_function(read_integer).get());
	env.global_declare("read_number", env.new_native_function(read_number).get());
	env.global_declare("read_string", env.new_native_function(read_string).get());
	env.global_declare("read_line", env.new_native_function(read_line).get());
}

#undef OP

} // namespace Interpreter
