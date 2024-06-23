#pragma once

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "value.hpp"

namespace Interpreter {

struct Stack {

	void start_frame(int size = 0);
	void end_frame();

	void start_region(int size = 0);
	void end_region();

	void push(Value ref);
	Value pop();

	Value& access(int offset);
	Value& frame_at(int offset);
	Span<Value> frame_range(int offset, int length);

	template<typename Function>
	void for_each(Function&& f) {
		for (auto const& v : m_stack) {
			f(v);
		}
	}

private:
	int m_frame_ptr {0};
	int m_stack_ptr {0};
	std::vector<Value> m_stack;
	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;
};

} // namespace Interpreter
