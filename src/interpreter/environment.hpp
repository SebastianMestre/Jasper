#pragma once

#include "../utils/interned_string.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct Environment {
	int m_frame_ptr {0};
	int m_stack_ptr {0};
	std::vector<Value*> m_stack;
	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;

	void start_stack_frame();
	void end_stack_frame();
	void start_stack_region();
	void end_stack_region();

	void push(Value* val);
	Value* pop_unsafe();
	gc_ptr<Value> pop();
};

} // namespace Interpreter
