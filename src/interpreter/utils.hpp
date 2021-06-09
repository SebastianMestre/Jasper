#pragma once

#include <cassert>

#include "value.hpp"

namespace Interpreter {

Handle value_of(Handle value);

template<typename T>
T* value_as(Handle h) {
	static_assert(std::is_base_of<GcCell, T>::value, "T is not a subclass of GcCell");
	return value_of(h).get_cast<T>();
}

} // namespace Interpreter
