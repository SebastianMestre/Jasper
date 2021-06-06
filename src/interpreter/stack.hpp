#pragma once

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct Stack {
	int m_frame_ptr {0};
	int m_stack_ptr {0};
	std::vector<Handle> m_stack;
	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;

	void start_stack_frame(int start);
	void start_stack_frame();
	void end_stack_frame();
	void start_stack_region(int start);
	void start_stack_region();
	void end_stack_region();

	void push(Handle ref);

	Handle pop_unsafe();

	Handle& access(int offset);
	Handle& frame_at(int offset);
	Span<Handle> frame_range(int offset, int length);
};

} // namespace Interpreter
