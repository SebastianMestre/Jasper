#pragma once

#include "value_tag.hpp"

namespace Interpreter {

struct GcCell {
  protected:
	ValueTag m_tag;

  public:
	bool m_visited = false;
	int m_cpp_refcount = 0;

	GcCell(ValueTag type)
	    : m_tag(type) {}

	ValueTag type() const {
		return m_tag;
	}

	virtual ~GcCell() = default;
};

void gc_visit(GcCell*);

} // namespace Interpreter
