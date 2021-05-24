#pragma once

#include <iostream>
#include <string>

#include "../interpreter/execute.hpp"
#include "../interpreter/value.hpp"

namespace Assert {

ExitStatus of_type(Interpreter::Value* rv, ValueTag v_type) {

	if (!rv)
		return ExitStatus::NullError;

	if (rv->type() != v_type)
		return ExitStatus::TypeError;

	return ExitStatus::Ok;
}

template <typename RuntimeValue, typename NativeValue>
ExitStatus scalar_equals(
    Interpreter::Value* rv, const ValueTag v_type, const NativeValue& expected) {
	ExitStatus fail = of_type(rv, v_type);

	if (ExitStatus::Ok != fail)
		return fail;

	if ((static_cast<RuntimeValue*>(rv))->m_value != expected)
		return ExitStatus::ValueError;

	return ExitStatus::Ok;
}

ExitStatus equals(Interpreter::Value* rv, std::string const& expected) {
	return scalar_equals<Interpreter::String, std::string>(
	    rv, ValueTag::String, expected);
}

ExitStatus equals(Interpreter::Value* rv, int expected) {
	return scalar_equals<Interpreter::Integer, int>(
	    rv, ValueTag::Integer, expected);
}

ExitStatus equals(Interpreter::Value* rv, float expected) {
	return scalar_equals<Interpreter::Float, float>(rv, ValueTag::Float, expected);
}

// NOTE: allows literals to be used (e.g. 3.5), may need to change in the future?
ExitStatus equals(Interpreter::Value* rv, double expected) {
	return equals(rv, float(expected));
}

ExitStatus is_true(Interpreter::Value* rv) {
	return scalar_equals<Interpreter::Boolean, bool>(rv, ValueTag::Boolean, true);
}

ExitStatus is_false(Interpreter::Value* rv) {
	return scalar_equals<Interpreter::Boolean, bool>(rv, ValueTag::Boolean, false);
}

ExitStatus is_null(Interpreter::Value* rv) {
	return of_type(rv, ValueTag::Null);
}

ExitStatus array_of_size(Interpreter::Value* rv, unsigned int size) {
	ExitStatus fail = of_type(rv, ValueTag::Array);

	if (ExitStatus::Ok != fail)
		return fail;

	if ((static_cast<Interpreter::Array*>(rv))->m_value.size() != size)
		return ExitStatus::ValueError;

	return ExitStatus::Ok;
}

} // namespace Assert
