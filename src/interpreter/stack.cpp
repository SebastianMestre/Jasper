#include "stack.hpp"

#include <cassert>

namespace Interpreter {

void Stack::start_frame(int size) {
	start_region(size);

	m_fp_stack.push_back(m_frame_ptr);
	m_frame_ptr = m_stack_ptr - size;
}

void Stack::end_frame(){
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

	m_stack.resize(m_stack_ptr);
}

void Stack::push(Value ref){
	m_stack.push_back(ref);
	m_stack_ptr += 1;
}

Value Stack::pop_unsafe() {
	Value result = m_stack.back();
	m_stack.pop_back();
	m_stack_ptr -= 1;
	return result;
}

Value& Stack::access(int offset) {
	return m_stack[m_stack_ptr - 1 - offset];
}

Value& Stack::frame_at(int offset) {
	assert(m_frame_ptr + offset >= 0);
	assert(m_frame_ptr + offset < m_stack_ptr);
	return m_stack[m_frame_ptr + offset];
}

Span<Value> Stack::frame_range(int offset, int length) {
	if (length > 0) {
		assert(m_frame_ptr + offset >= 0);
		assert(m_frame_ptr + offset + length <= m_stack_ptr);
	}
	auto start_address = &m_stack[m_frame_ptr + offset];
	return {start_address, length};
}

} // namespace Interpreter
