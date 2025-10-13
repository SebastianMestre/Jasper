#include "../utils/span.hpp"
#include "../log/log.hpp"
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
	(lhs).as<type>()->m_value op (rhs).as<type>()->m_value

#define OP_(field, lhs, op, rhs)                                               \
	Value {(lhs).field op (rhs).field}

using ArgsType = Span<Value>;

static void assert_argument_count(ArgsType v, int expected, const std::string& function_name) {
	if (v.size() != expected) {
		Log::internal_error() << "Typechecker failed to catch wrong argument count for " << function_name
		                      << ", expected " << expected << ", but got " << v.size();
	}
}

static void runtime_check_variadic(ArgsType v, int expected, const std::string& function_name) {
	if (v.size() < expected) {
		Log::fatal() << "Runtime check failed for function " << function_name
		             << ": wrong argument count, expected at least " << expected
		             << ", but got " << v.size();
	}
}

void assert_type_equality(Value& lhs, Value& rhs, const std::string& operation) {
	if (lhs.type() != rhs.type()) {
		Log::internal_error() << "Typechecker failed to catch type mismatch in " << operation;
	}
}

[[noreturn]] static void assert_valid_operation_type(Value& value, const std::string& operation) {
	Log::internal_error() << "Typechecker failed to catch invalid " << operation << " of type "
	                      << value.type_string();
}

// print(vals...) prints the values in vals
Value print(ArgsType v, Interpreter& e) {
	for (auto value : v)
		print(value);
	return e.null();
}

// array_append(arr, vals...) appends the values in vals to the array
Value array_append(ArgsType v, Interpreter& e) {
	runtime_check_variadic(v, 1, "array_append");
	Array* array = v[0].as<Array>();
	for (unsigned int i = 1; i < v.size(); i++) {
		array->append(v[i]);
	}
	return Value {array};
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Value array_extend(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "array_extend");
	Array* arr1 = v[0].as<Array>();
	Array* arr2 = v[1].as<Array>();
	arr1->m_value.insert(
	    arr1->m_value.end(), arr2->m_value.begin(), arr2->m_value.end());
	return Value {arr1};
}

// size(array) returns the size of the array
Value size(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 1, "size");
	Array* array = v[0].as<Array>();

	return Value {int(array->m_value.size())};
}

// array_join(array, string) returns a string with
// the array values separated by the string element
Value array_join(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "array_join");
	Array* array = v[0].as<Array>();
	String* sep = v[1].as<String>();
	std::stringstream result;
	for (unsigned int i = 0; i < array->m_value.size(); i++) {
		if (i > 0) result << sep->m_value;
		result << array->m_value[i].get_integer();
	}
	return Value{e.m_gc->new_string_raw(result.str())};
}

Value value_add(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_add");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_add");
	switch (lhs.type()) {
	case ValueTag::Integer:
		return OP_(as_integer, lhs, +, rhs);
	case ValueTag::Float:
		return OP_(as_float, lhs, +, rhs);
	case ValueTag::String:
		return Value {e.m_gc->new_string_raw(OP(String, lhs, +, rhs))};
	default:
		assert_valid_operation_type(lhs, "addition");
	}
}

Value value_sub(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_sub");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_sub");
	switch (lhs.type()) {
	case ValueTag::Integer:
		return {OP_(as_integer, lhs, -, rhs)};
	case ValueTag::Float:
		return {OP_(as_float, lhs, -, rhs)};
	default:
		assert_valid_operation_type(lhs, "subtraction");
	}
}

Value value_mul(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_mul");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_mul");
	switch (lhs.type()) {
	case ValueTag::Integer:
		return {OP_(as_integer, lhs, *, rhs)};
	case ValueTag::Float:
		return {OP_(as_float, lhs, *, rhs)};
	default:
		assert_valid_operation_type(lhs, "multiplication");
	}
}

Value value_div(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_div");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_div");
	switch (lhs.type()) {
	case ValueTag::Integer:
		return {OP_(as_integer, lhs, /, rhs)};
	case ValueTag::Float:
		return {OP_(as_float, lhs, /, rhs)};
	default:
		assert_valid_operation_type(lhs, "division");
	}
}

Value value_logicand(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_logicand");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_logicand");
	if (lhs.type() == ValueTag::Boolean and rhs.type() == ValueTag::Boolean)
		return OP_(as_boolean, lhs, &&, rhs);
	assert_valid_operation_type(lhs, "logical and operation");
}

Value value_logicor(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_logicor");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_logicor");
	if (lhs.type() == ValueTag::Boolean and rhs.type() == ValueTag::Boolean)
		return OP_(as_boolean, lhs, ||, rhs);
	assert_valid_operation_type(lhs, "logical or operation");
}

Value value_logicxor(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_logicxor");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_logicxor");
	if (lhs.type() == ValueTag::Boolean and rhs.type() == ValueTag::Boolean)
		return OP_(as_boolean, lhs, !=, rhs);
	assert_valid_operation_type(lhs, "exclusive or operation");
}

Value value_equals(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_equals");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_equals");
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
	default:
		assert_valid_operation_type(lhs, "equility comparison");
	}
}

Value value_not_equals(ArgsType v, Interpreter& e) {
	bool b = value_equals(v, e).as_boolean;
	return Value {bool(!b)};
}

Value value_less(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 2, "value_less");
	auto lhs = v[0];
	auto rhs = v[1];
	assert_type_equality(lhs, rhs, "value_less");
	switch (lhs.type()) {
	case ValueTag::Integer:
		return OP_(as_integer, lhs, <, rhs);
	case ValueTag::Float:
		return OP_(as_float, lhs, <, rhs);
	case ValueTag::String:
		return Value {OP(String, lhs, <, rhs)};
	default:
		assert_valid_operation_type(lhs, "comparison");
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

Value read_integer(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 0, "read_integer");
	int result;
	std::cin >> result; // TODO: error handling
	return Value {result};
}

Value read_number(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 0, "read_number");
	float result;
	std::cin >> result; // TODO: error handling
	return Value {result};
}

Value read_line(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 0, "read_line");
	std::string result;
	std::getline(std::cin, result); // TODO: error handling
	return Value {e.m_gc->new_string_raw(std::move(result))};
}

Value read_string(ArgsType v, Interpreter& e) {
	assert_argument_count(v, 0, "read_string");
	std::string result;
	std::cin >> result; // TODO: error handling
	return Value {e.m_gc->new_string_raw(std::move(result))};
}

void declare_native_functions(Interpreter& env) {
	auto declare = [&](char const* name, NativeFunction* func) {
		env.global_declare(name, Value {func});
	};

	env.global_declare("int", env.null());
	env.global_declare("float", env.null());

	declare("print", print);
	declare("array_append", array_append);
	declare("array_extend", array_extend);
	declare("size", size);
	declare("array_join", array_join);
	declare("+", value_add);
	declare("-", value_sub);
	declare("*", value_mul);
	declare("/", value_div);
	declare("<", value_less);
	declare(">=", value_greater_or_equal);
	declare(">", value_greater);
	declare("<=", value_less_or_equal);
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
