#pragma once

#include <cassert>

#include "value.hpp"

namespace Interpreter {

Value value_of(Value value);

template<typename T>
T* value_as(Value h) {
	static_assert(std::is_base_of<GcCell, T>::value, "T is not a subclass of GcCell");
	return value_of(h).get_cast<T>();
}

} // namespace Interpreter
