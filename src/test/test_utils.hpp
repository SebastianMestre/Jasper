#pragma once

#include <iostream>
#include <string>

#include "../interpreter/execute.hpp"
#include "../interpreter/value.hpp"

namespace Assert {

using Interpreter::Value;

namespace detail {

ExitStatus of_type(Interpreter::Value h, ValueTag tag) {
	if (h.type() != tag)
		return ExitStatus::TypeError;

	return ExitStatus::Ok;
}

template <typename RuntimeValue, typename NativeValue>
ExitStatus scalar_equals(Value rv, const ValueTag v_type, const NativeValue& expected) {
	ExitStatus fail = of_type(rv, v_type);

	if (ExitStatus::Ok != fail)
		return fail;

	auto value = rv.get_cast<RuntimeValue>();
	if (value->m_value != expected)
		return ExitStatus::ValueError;

	return ExitStatus::Ok;
}

template <typename NativeValue, typename Getter>
ExitStatus scalar_equals_fn(
    Value rv,
    const ValueTag v_type,
    const NativeValue& expected,
    Getter getter
) {
	ExitStatus fail = of_type(rv, v_type);

	if (ExitStatus::Ok != fail)
		return fail;

	if (getter(rv) != expected)
		return ExitStatus::ValueError;

	return ExitStatus::Ok;
}

ExitStatus bool_equals(Value rv, bool expected) {
	return scalar_equals_fn(
	    rv, ValueTag::Boolean, expected, [](Value h) { return h.as_boolean; });
}

} // namespace detail

ExitStatus equals(Value rv, std::string const& expected) {
	return detail::scalar_equals<Interpreter::String, std::string>(rv, ValueTag::String, expected);
}

ExitStatus equals(Value rv, int expected) {
	return detail::scalar_equals_fn(
	    rv, ValueTag::Integer, expected, [](Value h) { return h.as_integer; });
}

ExitStatus equals(Value rv, float expected) {
	return detail::scalar_equals_fn(
	    rv, ValueTag::Float, expected, [](Value h) { return h.as_float; });
}

// NOTE: allows literals to be used (e.g. 3.5), may need to change in the future?
ExitStatus equals(Value rv, double expected) {
	return equals(rv, float(expected));
}

ExitStatus is_true(Value rv) {
	return detail::bool_equals(rv, true);
}

ExitStatus is_false(Value rv) {
	return detail::bool_equals(rv, false);
}

ExitStatus is_null(Value rv) {
	return detail::of_type(rv, ValueTag::Null);
}

ExitStatus array_of_size(Value rv, unsigned int size) {
	return ExitStatus::Ok;
	ExitStatus fail = detail::of_type(rv, ValueTag::Array);

	if (ExitStatus::Ok != fail)
		return fail;

	if (rv.get_cast<Interpreter::Array>()->m_value.size() != size)
		return ExitStatus::ValueError;

	return ExitStatus::Ok;
}

} // namespace Assert
