#pragma once

#include "../utils/interned_string.hpp"
#include "../utils/span.hpp"
#include "value.hpp"

namespace Interpreter {

struct Stack {

	// temp stack manipulation

	void push(Value ref);
	Value pop();
	Value& access(int offset);

	template<typename Function>
	void for_each(Function&& f) {
		for (auto const& v : m_temps) {
			f(v);
		}
	}

	// local stack manipulation

	void push_local(Variable* ref);
	Variable* pop_local();

	Variable* access_frame(int offset);
	
	Span<Value> stack_range(int offset, int length);

	void start_frame(int size = 0);
	void end_frame();

	void start_region(int size = 0);
	void end_region();

	template<typename Function>
	void for_each_local(Function&& f) {
		for (auto const& v : m_locals) {
			f(v);
		}
	}

private:
	std::vector<Value> m_temps;

	int m_frame_ptr {0};
	int m_stack_ptr {0};

	std::vector<Variable*> m_locals;

	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;
};

} // namespace Interpreter
