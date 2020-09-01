#include "utils.hpp"

#include "value.hpp"

namespace Interpreter {

Value* unboxed(Value* value) {
	if (!value)
		return value;

	if (value->type() != value_type::Reference)
		return value;

	// try unboxing recursively?
	auto ref = static_cast<Reference*>(value);
	return ref->m_value;
}

}
