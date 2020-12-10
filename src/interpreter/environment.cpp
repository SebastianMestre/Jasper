#include "environment.hpp"

#include <cassert>

namespace Interpreter {

void Environment::start_stack_frame() {
	m_fp_stack.push_back(m_frame_ptr);
	m_sp_stack.push_back(m_stack_ptr);
	m_frame_ptr = m_stack_ptr;
}

void Environment::end_stack_frame(){
	m_frame_ptr = m_fp_stack.back();
	m_fp_stack.pop_back();

	m_stack_ptr = m_sp_stack.back();
	m_sp_stack.pop_back();

	m_stack.resize(m_stack_ptr);
}

void Environment::start_stack_region() {
	m_sp_stack.push_back(m_stack_ptr);
}

void Environment::end_stack_region() {
	m_stack_ptr = m_sp_stack.back();
	m_sp_stack.pop_back();

	m_stack.resize(m_stack_ptr);
}

void Environment::push(Value* ref){
	m_stack.push_back(ref);
	m_stack_ptr += 1;
}

Value* Environment::pop_unsafe() {
	Value* result = m_stack.back();
	m_stack.pop_back();
	m_stack_ptr -= 1;
	return result;
}

gc_ptr<Value> Environment::pop() {
	gc_ptr<Value> result = {m_stack.back()};
	m_stack.pop_back();
	m_stack_ptr -= 1;
	return result;
}

} // namespace Interpreter
