#include "utils.hpp"

#include "value.hpp"

namespace Interpreter {

Value value_of(Value h) {
	if (!is_heap_type(h.type()))
		return h;

	if (h.type() != ValueTag::Reference)
		return h;

	// try unboxing recursively?
	auto ref = h.get_cast<Reference>();
	return ref->m_value;
}

} // namespace Interpreter
