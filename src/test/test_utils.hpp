#pragma once

#include <iostream>
#include <string>

#include "../interpreter/environment.hpp"
#include "../interpreter/execute.hpp"
#include "../interpreter/value.hpp"

namespace Assert {

exit_status_type of_type(Interpreter::Value* rv, value_type v_type) {

	if (!rv)
		return exit_status_type::NullError;

	if (rv->type() != v_type)
		return exit_status_type::TypeError;

	return exit_status_type::Ok;
}

template <typename RuntimeValue, typename NativeValue>
exit_status_type scalar_equals(
    Interpreter::Value* rv, const value_type v_type, const NativeValue& expected) {
	exit_status_type fail = of_type(rv, v_type);

	if (exit_status_type::Ok != fail)
		return fail;

	if ((static_cast<RuntimeValue*>(rv))->m_value != expected)
		return exit_status_type::ValueError;

	return exit_status_type::Ok;
}

exit_status_type equals(Interpreter::Value* rv, std::string const& expected) {
	return scalar_equals<Interpreter::String, std::string>(
	    rv, value_type::String, expected);
}

exit_status_type equals(Interpreter::Value* rv, int expected) {
	return scalar_equals<Interpreter::Integer, int>(
	    rv, value_type::Integer, expected);
}

exit_status_type equals(Interpreter::Value* rv, float expected) {
	return scalar_equals<Interpreter::Float, float>(rv, value_type::Float, expected);
}

// NOTE: allows literals to be used (e.g. 3.5), may need to change in the future?
exit_status_type equals(Interpreter::Value* rv, double expected) {
	return equals(rv, float(expected));
}

exit_status_type is_true(Interpreter::Value* rv) {
	return scalar_equals<Interpreter::Boolean, bool>(rv, value_type::Boolean, true);
}

exit_status_type is_false(Interpreter::Value* rv) {
	return scalar_equals<Interpreter::Boolean, bool>(rv, value_type::Boolean, false);
}

exit_status_type is_null(Interpreter::Value* rv) {
	return of_type(rv, value_type::Null);
}

exit_status_type array_of_size(Interpreter::Value* rv, unsigned int size) {
	exit_status_type fail = of_type(rv, value_type::Array);

	if (exit_status_type::Ok != fail)
		return fail;

	if ((static_cast<Interpreter::Array*>(rv))->m_value.size() != size)
		return exit_status_type::ValueError;

	return exit_status_type::Ok;
}

} // Assert
