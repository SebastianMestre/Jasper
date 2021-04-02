#include "interpreter.hpp"

#include <cassert>

#include "error.hpp"
#include "memory_manager.hpp"
#include "utils.hpp"

namespace Interpreter {

void Scope::declare(const Identifier& i, Reference* v) {
	auto insertion_result = m_declarations.insert({i, v});
	assert(insertion_result.second);
}

Reference* Scope::access(const Identifier& i) {
	auto v = m_declarations.find(i);

	if (v == m_declarations.end()) {
		// TODO: error
		return nullptr;
	}

	return v->second;
}

void Interpreter::global_declare_direct(const Identifier& i, Reference* r) {
	m_global_scope.declare(i, r);
}

void Interpreter::global_declare(const Identifier& i, Value* v) {
	if (v->type() == ValueTag::Reference)
		assert(0 && "declared a reference!");
	auto r = new_reference(v);
	global_declare_direct(i, r.get());
}

void Interpreter::global_declare(const Identifier& i, gc_ptr<Value> v) {
	global_declare(i, v.get());
}

Reference* Interpreter::global_access(const Identifier& i) {
	return m_global_scope.access(i);
}


void Interpreter::save_return_value(Value* v) {
	// check if not stepping on another value
	assert(!m_return_value);
	m_return_value = v;
}

Value* Interpreter::fetch_return_value() {
	Value* rv = m_return_value;
	m_return_value = nullptr;
	return rv;
}

void Interpreter::assign(Value* dst, Value* src) {
	// NOTE: copied by reference, matters if rhs is actually a reference
	// TODO: change in another pr, perhaps adding Interpreter::copy_value?
	as<Reference>(dst)->m_value = value_of(src);
}


void Interpreter::run_gc() {
	m_gc->m_gc.unmark_all();
	m_gc->m_gc.mark_roots();

	for (auto p : m_stack.m_stack)
		gc_visit(p);

	for (auto& p : m_global_scope.m_declarations)
		gc_visit(p.second);

	m_gc->m_gc.sweep();
}

void Interpreter::run_gc_if_needed(){
	if (m_gc->m_gc.size() >= 2*m_gc_size_on_last_pass) {
		run_gc();
		m_gc_size_on_last_pass = m_gc->m_gc.size();
	}
}


Null* Interpreter::null() {
	return m_gc->null();
}

gc_ptr<Reference> Interpreter::new_reference(Value* v) {
	assert(
	    v->type() != ValueTag::Reference &&
	    "References to references are not allowed.");
	auto result = m_gc->alloc<Reference>(v);
	run_gc_if_needed();
	return result;
}

} // namespace Interpreter
