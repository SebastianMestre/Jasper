#pragma once

#include <string>
#include <iostream>

#include "value.hpp"
#include "environment.hpp"
#include "execute.hpp"

namespace Assert {

test_type of_type(Type::Value* rv, value_type v_type) {

	if (!rv)
		return test_type::NullError;

	if (rv->type() != v_type)
		return test_type::TypeError;

	return test_type::Ok;
}

// CV - C++ scalar type of the value to check against
// RV - Type::Value counterpart
template <typename RV, typename CV>
test_type scalar_equals(Type::Value* rv, const value_type v_type, const CV& expected) {
	test_type fail = of_type(rv, v_type);

	if (test_type::Ok != fail)
		return fail;

	if ((static_cast<RV*>(v))->m_value != value)
		return test_type::TypeError;

	return test_type::Ok;
}

test_type equals(Type::Value* rv, std::string const& expected) {
	return scalar_equals<Type::String, std::string>(rv, value_type::String, expected);
}

test_type equals(Type::Value* rv, int expected) {
	return scalar_equals<Type::Integer, int>(rv, value_type::Integer, expected);
}

test_type equals(Type::Value* rv, float expected) {
	return scalar_equals<Type::Float, float>(rv, value_type::Float, expected);
}

// NOTE: allows literals to be used (e.g. 3.5), may need to change in the future?
test_type equals(Type::Value* rv, double expected) {
	return equals(rv, float(expected));
}

test_type is_true(Type::Value* rv) {
	return scalar_equals<Type::Boolean, bool>(rv, value_type::Boolean, true);
}

test_type is_false(Type::Value* rv) {
	return scalar_equals<Type::Boolean, bool>(rv, value_type::Boolean, false);
}

test_type is_null(Type::Value* rv) {
	return of_type(rv, value_type::Null);
}

test_type array_of_size(Type::Value* rv, unsigned int size) {
	test_type fail = of_type(rv, value_type::Array);

	if (test_type::Ok != fail)
		return fail;

	if ((static_cast<Type::Array*>(result))->m_value.size() != size)
		return test_type::ValueError;

	return test_type::Ok;
}

} // Assert
