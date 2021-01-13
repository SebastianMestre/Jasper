#pragma once

#include <iostream>
#include <string>

#include "../interpreter/execute.hpp"
#include "../interpreter/value.hpp"

namespace Assert {

ExitStatusTag of_type(Interpreter::Value* rv, ValueTag v_type) {

	if (!rv)
		return ExitStatusTag::NullError;

	if (rv->type() != v_type)
		return ExitStatusTag::TypeError;

	return ExitStatusTag::Ok;
}

template <typename RuntimeValue, typename NativeValue>
ExitStatusTag scalar_equals(
    Interpreter::Value* rv, const ValueTag v_type, const NativeValue& expected) {
	ExitStatusTag fail = of_type(rv, v_type);

	if (ExitStatusTag::Ok != fail)
		return fail;

	if ((static_cast<RuntimeValue*>(rv))->m_value != expected)
		return ExitStatusTag::ValueError;

	return ExitStatusTag::Ok;
}

ExitStatusTag equals(Interpreter::Value* rv, std::string const& expected) {
	return scalar_equals<Interpreter::String, std::string>(
	    rv, ValueTag::String, expected);
}

ExitStatusTag equals(Interpreter::Value* rv, int expected) {
	return scalar_equals<Interpreter::Integer, int>(
	    rv, ValueTag::Integer, expected);
}

ExitStatusTag equals(Interpreter::Value* rv, float expected) {
	return scalar_equals<Interpreter::Float, float>(rv, ValueTag::Float, expected);
}

// NOTE: allows literals to be used (e.g. 3.5), may need to change in the future?
ExitStatusTag equals(Interpreter::Value* rv, double expected) {
	return equals(rv, float(expected));
}

ExitStatusTag is_true(Interpreter::Value* rv) {
	return scalar_equals<Interpreter::Boolean, bool>(rv, ValueTag::Boolean, true);
}

ExitStatusTag is_false(Interpreter::Value* rv) {
	return scalar_equals<Interpreter::Boolean, bool>(rv, ValueTag::Boolean, false);
}

ExitStatusTag is_null(Interpreter::Value* rv) {
	return of_type(rv, ValueTag::Null);
}

ExitStatusTag array_of_size(Interpreter::Value* rv, unsigned int size) {
	ExitStatusTag fail = of_type(rv, ValueTag::Array);

	if (ExitStatusTag::Ok != fail)
		return fail;

	if ((static_cast<Interpreter::Array*>(rv))->m_value.size() != size)
		return ExitStatusTag::ValueError;

	return ExitStatusTag::Ok;
}

} // namespace Assert
