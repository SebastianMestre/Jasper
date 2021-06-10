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

#define OP(type, lhs, op, rhs)                                                 \
	(lhs).get_cast<type>()->m_value op (rhs).get_cast<type>()->m_value

#define OP_(field, lhs, op, rhs)                                               \
	Value {(lhs).field op (rhs).field}

using ArgsType = Span<Value>;

// print(vals...) prints the values or references in vals
Value print(ArgsType v, Interpreter& e) {
	for (auto value : v)
		print(value);
	return e.null();
}

// array_append(arr, vals...) appends the values in vals to the array
Value array_append(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() > 0);
	Array* array = value_as<Array>(v[0]);
	for (unsigned int i = 1; i < v.size(); i++) {
		array->append(e.new_reference(value_of(v[i])).get());
	}
	return Value {array};
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Value array_extend(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	Array* arr1 = value_as<Array>(v[0]);
	Array* arr2 = value_as<Array>(v[1]);
	arr1->m_value.insert(
	    arr1->m_value.end(), arr2->m_value.begin(), arr2->m_value.end());
	return Value {arr1};
}

// size(array) returns the size of the array
Value size(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() == 1);
	Array* array = value_as<Array>(v[0]);

	return Value {int(array->m_value.size())};
}

// array_join(array, string) returns a string with
// the array values separated by the string element
Value array_join(ArgsType v, Interpreter& e) {
	// TODO make it more general
	// TODO proper error handling
	assert(v.size() == 2);
	Array* array = value_as<Array>(v[0]);
	String* sep = value_as<String>(v[1]);
	std::stringstream result;
	for (unsigned int i = 0; i < array->m_value.size(); i++) {
		if (i > 0) result << sep->m_value;
		result << array->m_value[i]->m_value.get_integer();
	}
	return Value{e.m_gc->new_string_raw(result.str())};
}

// array_at(array, int i) returns the i-th element of the given array
Value array_at(ArgsType v, Interpreter& e) {
	// TODO proper error handling
	assert(v.size() == 2);
	Array* array = value_as<Array>(v[0]);
	int index = v[1].get_integer();
	assert(index >= 0);
	assert(index < array->m_value.size());
	return Value{array->m_value[index]};
}

Value value_add(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	assert(lhs.type() == rhs.type());
	switch (lhs.type()) {
	case ValueTag::Integer:
		return OP_(as_integer, lhs, +, rhs);
	case ValueTag::Float:
		return OP_(as_float, lhs, +, rhs);
	case ValueTag::String:
		return Value {e.m_gc->new_string_raw(OP(String, lhs, +, rhs))};
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_string[static_cast<int>(lhs.type())];
		assert(0);
	}
}

Value value_sub(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	assert(lhs.type() == rhs.type());
	switch (lhs.type()) {
	case ValueTag::Integer:
		return {OP_(as_integer, lhs, -, rhs)};
	case ValueTag::Float:
		return {OP_(as_float, lhs, -, rhs)};
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_string[static_cast<int>(lhs.type())];
		assert(0);
	}
}

Value value_mul(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	assert(lhs.type() == rhs.type());
	switch (lhs.type()) {
	case ValueTag::Integer:
		return {OP_(as_integer, lhs, *, rhs)};
	case ValueTag::Float:
		return {OP_(as_float, lhs, *, rhs)};
	default:
		std::cerr << "ERROR: can't multiply values of type "
		          << value_string[static_cast<int>(lhs.type())];
		assert(0);
	}
}

Value value_div(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	assert(lhs.type() == rhs.type());
	switch (lhs.type()) {
	case ValueTag::Integer:
		return {OP_(as_integer, lhs, /, rhs)};
	case ValueTag::Float:
		return {OP_(as_float, lhs, /, rhs)};
	default:
		std::cerr << "ERROR: can't divide values of type "
		          << value_string[static_cast<int>(lhs.type())];
		assert(0);
	}
}

Value value_logicand(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	if (lhs.type() == ValueTag::Boolean and rhs.type() == ValueTag::Boolean)
		return OP_(as_boolean, lhs, &&, rhs);
	std::cerr << "ERROR: logical and operator not defined for types "
	          << value_string[static_cast<int>(lhs.type())] << " and "
	          << value_string[static_cast<int>(rhs.type())];
	assert(0);
}

Value value_logicor(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	if (lhs.type() == ValueTag::Boolean and rhs.type() == ValueTag::Boolean)
		return OP_(as_boolean, lhs, ||, rhs);
	std::cerr << "ERROR: logical or operator not defined for types "
	          << value_string[static_cast<int>(lhs.type())] << " and "
	          << value_string[static_cast<int>(rhs.type())];
	assert(0);
}

Value value_logicxor(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	if (lhs.type() == ValueTag::Boolean and rhs.type() == ValueTag::Boolean)
		return OP_(as_boolean, lhs, !=, rhs);
	std::cerr << "ERROR: exclusive or operator not defined for types "
	          << value_string[static_cast<int>(lhs.type())] << " and "
	          << value_string[static_cast<int>(rhs.type())];
	assert(0);
}

Value value_equals(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	assert(lhs.type() == rhs.type());

	switch (lhs.type()) {
	case ValueTag::Null:
		return Value {true};
	case ValueTag::Integer:
		return OP_(as_integer, lhs, ==, rhs);
	case ValueTag::Float:
		return OP_(as_float, lhs, ==, rhs);
	case ValueTag::String:
		return Value {OP(String, lhs, ==, rhs)};
	case ValueTag::Boolean:
		return OP_(as_boolean, lhs, ==, rhs);
	default: {
		std::cerr << "ERROR: can't compare equality of types "
		          << value_string[static_cast<int>(lhs.type())] << " and "
		          << value_string[static_cast<int>(rhs.type())];
		assert(0);
	}
	}
}

Value value_not_equals(ArgsType v, Interpreter& e) {
	bool b = value_equals(v, e).as_boolean;
	return Value {bool(!b)};
}

Value value_less(ArgsType v, Interpreter& e) {
	auto lhs = value_of(v[0]);
	auto rhs = value_of(v[1]);

	assert(lhs.type() == rhs.type());

	switch (lhs.type()) {
	case ValueTag::Integer:
		return OP_(as_integer, lhs, <, rhs);
	case ValueTag::Float:
		return OP_(as_float, lhs, <, rhs);
	case ValueTag::String:
		return Value {OP(String, lhs, <, rhs)};
	default:
		std::cerr << "ERROR: can't compare values of type "
		          << value_string[static_cast<int>(lhs.type())];
		assert(0);
	}
}

Value value_greater_or_equal(ArgsType v, Interpreter& e) {
	bool b = value_less(v, e).as_boolean;
	return Value {bool(!b)};
}

Value value_greater(ArgsType v, Interpreter& e) {
	Value args[2] = {v[1], v[0]}; // arguments are swapped
	return value_less(Span<Value> {args, 2}, e);
}

Value value_less_or_equal(ArgsType v, Interpreter& e) {
	bool b = value_greater(v, e).as_boolean;
	return Value {bool(!b)};
}

Value value_assign(ArgsType v, Interpreter& e) {
	e.assign(v[0], v[1]);
	return e.null();
}

Value read_integer(ArgsType v, Interpreter& e) {
	// TODO: error handling
	int result;
	std::cin >> result;
	return Value {result};
}

Value read_number(ArgsType v, Interpreter& e) {
	// TODO: error handling
	float result;
	std::cin >> result;
	return Value {result};
}

Value read_line(ArgsType v, Interpreter& e) {
	// TODO: error handling
	std::string result;
	std::getline(std::cin, result);
	return Value {e.m_gc->new_string_raw(std::move(result))};
}

Value read_string(ArgsType v, Interpreter& e) {
	// TODO: error handling
	std::string result;
	std::cin >> result;
	return Value {e.m_gc->new_string_raw(std::move(result))};
}

void declare_native_functions(Interpreter& env) {
	auto declare = [&](char const* name, NativeFunction* func) {
		env.global_declare(name, Value {func});
	};

	declare("print", print);
	declare("array_append", array_append);
	declare("array_extend", array_extend);
	declare("size", size);
	declare("array_join", array_join);
	declare("array_at", array_at);
	declare("+", value_add);
	declare("-", value_sub);
	declare("*", value_mul);
	declare("/", value_div);
	declare("<", value_less);
	declare(">=", value_greater_or_equal);
	declare(">", value_greater);
	declare("<=", value_less_or_equal);
	declare("=", value_assign);
	declare("==", value_equals);
	declare("!=", value_not_equals);
	declare("^^", value_logicxor);
	declare("&&", value_logicand);
	declare("||", value_logicor);

	// Input
	declare("read_integer", read_integer);
	declare("read_number", read_number);
	declare("read_string", read_string);
	declare("read_line", read_line);
}

#undef OP

} // namespace Interpreter
