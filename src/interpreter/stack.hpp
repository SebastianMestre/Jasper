#pragma once

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct Stack {

	void start_frame(int size = 0);
	void end_frame();

	void start_region(int size = 0);
	void end_region();

	void push(Value ref);
	Value pop_unsafe();

	Value& access(int offset);
	Value& frame_at(int offset);
	Span<Value> frame_range(int offset, int length);

	std::vector<Value> m_stack;
private:
	int m_frame_ptr {0};
	int m_stack_ptr {0};
	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;
};

} // namespace Interpreter
