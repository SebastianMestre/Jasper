#pragma once

#include <string>
#include <iostream>

#include "value.hpp"
#include "environment.hpp"
#include "execute.hpp"

namespace Assert {

// NOTE: We currently implement funcion evaluation in eval(AST::CallExpression)
// this means we need to create a call expression node to run the program.
// TODO: We need to clean this up
Type::Value* eval_expression(const std::string& expr, Type::Environment& env) {
	TokenArray ta;
	auto top_level_call_ast = parse_expression(expr, ta);
	auto top_level_call = TypedAST::get_unique(top_level_call_ast.m_result);

	return Type::unboxed(eval(top_level_call.get(), env));
}

// NOTE: Helper function for null and type checking.
// Not supposed to be used, as it does not print `Success`.
// Perhaps the printing of such string can be deprecated in the future.
int of_type(Type::Value* v, value_type v_type) {

	if(!v) {
		std::cerr << "ERROR: Null result\n";
		return 1;
	}

	if(v->type() != v_type) {
		// TODO: better error message
		std::cerr << "ERROR: Type discrepancy\n";
		return 2;
	}

	return 0;
}

// T - C++ scalar type of the value to check against
// U - Type::Value counterpart
template <typename T, typename U>
int scalar_equals(
    const std::string& expr, const value_type v_type, const T& value, Type::Environment& env
) {
	auto* result = eval_expression(expr, env);

	int fail = of_type(result, v_type);
	if (fail)
		return fail;

	auto* as_u = static_cast<U*>(result);
	if(as_u->m_value != value){
		std::cerr
			<< "ERROR:\n"
			<< "Expected: " << value << std::endl
			<< "Got: " << as_u->m_value << std::endl;
		return 3;
	}

	std::cout << "Success\n";
	return 0;
}

int equals(std::string const& expr, std::string const& value, Type::Environment& env) {
	return scalar_equals<std::string, Type::String>(expr, value_type::String, value, env);
}

int equals(std::string const& expr, int value, Type::Environment& env) {
	return scalar_equals<int, Type::Integer>(expr, value_type::Integer, value, env);
}

int equals(std::string const& expr, float value, Type::Environment& env) {
	return scalar_equals<float, Type::Float>(expr, value_type::Float, value, env);
}

// NOTE: allows literals to be used (e.g. 3.5), may need to change in the future?
int equals(std::string const& expr, double value, Type::Environment& env) {
	return equals(expr, float(value), env);
}

int is_true(std::string const& expr, Type::Environment& env) {
	return scalar_equals<bool, Type::Boolean>(expr, value_type::Boolean, true, env);
}

int is_false(std::string const& expr, Type::Environment& env) {
	return scalar_equals<bool, Type::Boolean>(expr, value_type::Boolean, false, env);
}

int is_null(std::string const& expr, Type::Environment& env) {
	auto* result = eval_expression(expr, env);

	int fail = of_type(result, value_type::Null);
	if (fail)
		return fail;

	std::cout << "Success\n";
	return 0;
}

int array_of_size(std::string const& expr, int size, Type::Environment& env) {
	assert(size >= 0);
	auto* result = Assert::eval_expression(expr, env);

	int fail = of_type(result, value_type::Array);
	if (fail)
		return fail;

	auto* array = static_cast<Type::Array*>(result);
	if (array->m_value.size() != size) {
		std::cerr
			<< "ERROR:\n"
			<< "Expected: array of size" << size << std::endl
			<< "Got: array of size " << array->m_value.size() << std::endl;
		return 3;
	}

	std::cout << "Success\n";
	return 0;
}

} // Assert
