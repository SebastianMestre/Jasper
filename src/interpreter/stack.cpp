#include "stack.hpp"

#include <cassert>

namespace Interpreter {

void Stack::start_frame(int size) {
	start_region(size);

	m_fp_stack.push_back(m_frame_ptr);
	m_frame_ptr = m_stack_ptr - size;
}

void Stack::end_frame() {
	m_frame_ptr = m_fp_stack.back();
	m_fp_stack.pop_back();

	end_region();
}

void Stack::start_region(int size) {
	m_sp_stack.push_back(m_stack_ptr - size);
}

void Stack::end_region() {
	m_stack_ptr = m_sp_stack.back();
	m_sp_stack.pop_back();

	m_locals.resize(m_stack_ptr);
}

void Stack::push(Value ref) {
	m_temps.push_back(ref);
}

Value Stack::pop() {
	auto result = m_temps.back();
	m_temps.pop_back();
	return result;
}

Value& Stack::access(int offset) {
	return m_temps[m_temps.size() - 1 - offset];
}

Variable* Stack::access_frame(int offset) {
	assert(m_frame_ptr + offset >= 0);
	assert(m_frame_ptr + offset < m_stack_ptr);
	return m_locals[m_frame_ptr + offset];
}

void Stack::push_local(Variable* ref) {
	m_locals.push_back(ref);
	m_stack_ptr += 1;
}

Variable* Stack::pop_local() {
	auto result = m_locals.back();
	m_locals.pop_back();
	m_stack_ptr -= 1;
	return result;
}

Span<Value> Stack::stack_range(int offset, int length) {
	if (length > 0) {
		assert(offset >= 0);
		assert(offset <= m_temps.size());
		assert(length >= 0);
		assert(length <= offset);
	}
	auto start_address = &m_temps[m_temps.size() - offset];
	return {start_address, length};
}

} // namespace Interpreter
