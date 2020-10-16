#include "environment.hpp"

#include <cassert>

#include "garbage_collector.hpp"

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

void Environment::save_return_value(Value* v) {
	// check if not stepping on another value
	assert(!m_return_value);
	m_return_value = v;
}

Value* Environment::fetch_return_value() {
	Value* rv = m_return_value;
	m_return_value = nullptr;
	return rv;
}


void Environment::global_declare_direct(const Identifier& i, Reference* r) {
	m_global_scope.declare(i, r);
}

void Environment::global_declare(const Identifier& i, Value* v) {
	if (v->type() == ValueTag::Reference)
		assert(0 && "declared a reference!");
	auto r = new_reference(v);
	global_declare_direct(i, r.get());
}

void Environment::global_declare(const Identifier& i, gc_ptr<Value> v) {
	global_declare(i, v.get());
}

Reference* Environment::global_access(const Identifier& i) {
	return m_global_scope.access(i);
}

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

gc_ptr<Value> Environment::pop(){
	gc_ptr<Value> result = {m_stack.back()};
	m_stack.pop_back();
	m_stack_ptr -= 1;
	return result;
}

void Environment::run_gc() {
	m_gc->unmark_all();
	m_gc->mark_roots();

	for (auto p : m_stack)
		gc_visit(p);

	for (auto& p : m_global_scope.m_declarations)
		gc_visit(p.second);

	m_gc->sweep();
}

void Environment::run_gc_if_needed(){
	if (m_gc->size() >= 2*m_gc_size_on_last_pass) {
		run_gc();
		m_gc_size_on_last_pass = m_gc->size();
	}
}

Null* Environment::null() {
	return m_gc->null();
}

void Environment::push_integer(int i) {
	push(m_gc->new_integer_raw(i));
	run_gc_if_needed();
}

void Environment::push_float(float f) {
	push(m_gc->new_float_raw(f));
	run_gc_if_needed();
}

void Environment::push_boolean(bool b) {
	push(m_gc->new_boolean_raw(b));
	run_gc_if_needed();
}

gc_ptr<String> Environment::new_string(std::string s) {
	auto result = m_gc->new_string(std::move(s));
	run_gc_if_needed();
	return result;
}

gc_ptr<Array> Environment::new_list(ArrayType elements) {
	auto result = m_gc->new_list(std::move(elements));
	run_gc_if_needed();
	return result;
}

gc_ptr<Object> Environment::new_object(ObjectType declarations) {
	auto result = m_gc->new_object(std::move(declarations));
	run_gc_if_needed();
	return result;
}

gc_ptr<Dictionary> Environment::new_dictionary(DictionaryType declarations) {
	auto result = m_gc->new_dictionary(std::move(declarations));
	run_gc_if_needed();
	return result;
}

gc_ptr<Function> Environment::new_function(FunctionType def, ObjectType s) {
	auto result = m_gc->new_function(def, std::move(s));
	run_gc_if_needed();
	return result;
}

gc_ptr<NativeFunction> Environment::new_native_function(NativeFunctionType* fptr) {
	auto result = m_gc->new_native_function(fptr);
	run_gc_if_needed();
	return result;
}

gc_ptr<Error> Environment::new_error(std::string e) {
	auto result = m_gc->new_error(e);
	run_gc_if_needed();
	return result;
}

gc_ptr<Reference> Environment::new_reference(Value* v) {
	assert(
	    v->type() != ValueTag::Reference &&
	    "References to references are not allowed.");
	auto result = m_gc->new_reference(v);
	run_gc_if_needed();
	return result;
}

} // namespace Interpreter
