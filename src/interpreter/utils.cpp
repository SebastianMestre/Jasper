#include "utils.hpp"

#include "value.hpp"

namespace Interpreter {

Value* value_of(Value* value) {
	if (!value)
		return value;

	if (value->type() != ValueTag::Reference)
		return value;

	// try unboxing recursively?
	auto ref = static_cast<Reference*>(value);
	return ref->m_value;
}

} // namespace Interpreter
