#pragma once

#include <cassert>

#include "value.hpp"

namespace Interpreter {

Handle value_of(Handle value);

template<typename T>
T* as(Handle h) {
	static_assert(std::is_base_of<Value, T>::value, "T is not a subclass of Value");
	assert(h.type() == type_data<T>::tag);
	T* result = h.get_cast<T>();
	assert(result);
	return result;
}

template<typename T>
T* value_as(Handle h) {
	static_assert(std::is_base_of<Value, T>::value, "T is not a subclass of Value");
	return as<T>(value_of(h));
}

} // namespace Interpreter
