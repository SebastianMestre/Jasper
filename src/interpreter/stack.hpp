#pragma once

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct Stack {
	int m_frame_ptr {0};
	int m_stack_ptr {0};
	std::vector<Value> m_stack;
	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;

	void start_stack_frame(int start);
	void start_stack_frame();
	void end_stack_frame();
	void start_stack_region(int start);
	void start_stack_region();
	void end_stack_region();

	void push(Value ref);

	Value pop_unsafe();

	Value& access(int offset);
	Value& frame_at(int offset);
	Span<Value> frame_range(int offset, int length);
};

} // namespace Interpreter
