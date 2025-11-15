#pragma once

#include "value_tag.hpp"

#include <cstdint>

namespace Interpreter {

struct GcCell {
protected:
	ValueTag m_tag;
	uint8_t gc_bits = 0;
public:

	GcCell(ValueTag type)
	    : m_tag(type) {}

	ValueTag type() const {
		return m_tag;
	}

	const char* type_string() const {
		return value_string[(int)m_tag];
	}

	void visit(int generation);
	void mark(int generation) { gc_bits = (generation & 1) | 2; }
	bool is_marked(int generation) const { return gc_bits == (generation & 1) | 2; }

	virtual ~GcCell() = default;
};

} // namespace Interpreter
