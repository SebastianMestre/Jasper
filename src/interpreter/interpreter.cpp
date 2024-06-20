#include "interpreter.hpp"

#include <cassert>

#include "error.hpp"
#include "garbage_collector.hpp"
#include "utils.hpp"

namespace Interpreter {

void GlobalScope::declare(const Identifier& i, Variable* v) {
	auto insertion_result = m_declarations.insert({i, v});
	assert(insertion_result.second);
}

Variable* GlobalScope::access(const Identifier& i) {
	auto v = m_declarations.find(i);

	if (v == m_declarations.end()) {
		// TODO: error
		return nullptr;
	}

	return v->second;
}

void Interpreter::global_declare_direct(const Identifier& i, Variable* r) {
	m_global_scope.declare(i, r);
}

void Interpreter::global_declare(const Identifier& i, Value v) {
	assert(v.type() != ValueTag::Variable);
	auto r = new_variable(v);
	global_declare_direct(i, r.get());
}

Variable* Interpreter::global_access(const Identifier& i) {
	return m_global_scope.access(i);
}


void Interpreter::save_return_value(Value v) {
	// ensure we are not in a return sequence already
	assert(!m_returning);
	m_returning = true;
	m_return_value = v;
}

Value Interpreter::fetch_return_value() {
	// ensure we were in a return sequence
	assert(m_returning);
	m_returning = false;
	Value rv = m_return_value;
	m_return_value = Value{};
	return rv;
}


void Interpreter::run_gc() {
	m_gc->unmark_all();
	m_gc->mark_roots();

	m_stack.for_each([](Value p) {
		if (is_heap_type(p.type())) {
			p.get()->visit();
		}
	});

	for (auto& p : m_global_scope.m_declarations)
		p.second->visit();

	m_gc->sweep();
}

void Interpreter::run_gc_if_needed(){
	if (m_gc->size() >= 2*m_gc_size_on_last_pass) {
		run_gc();
		m_gc_size_on_last_pass = m_gc->size();
	}
}


Value Interpreter::null() {
	return Value{nullptr};
}

void Interpreter::push_integer(int i) {
	m_stack.push(Value{i});
	run_gc_if_needed();
}

void Interpreter::push_float(float f) {
	m_stack.push(Value{f});
	run_gc_if_needed();
}

void Interpreter::push_boolean(bool b) {
	m_stack.push(Value{b});
	run_gc_if_needed();
}

void Interpreter::push_string(std::string s) {
	m_stack.push(Value{m_gc->new_string_raw(std::move(s))});
	run_gc_if_needed();
}

void Interpreter::push_variant_constructor(InternedString constructor) {
	m_stack.push(Value{m_gc->new_variant_constructor_raw(constructor)});
	run_gc_if_needed();
}

void Interpreter::push_record_constructor(std::vector<InternedString> keys) {
	m_stack.push(Value{m_gc->new_record_constructor_raw(std::move(keys))});
	run_gc_if_needed();
}

gc_ptr<Array> Interpreter::new_list(ArrayType elements) {
	auto result = m_gc->new_list(std::move(elements));
	run_gc_if_needed();
	return result;
}

gc_ptr<Record> Interpreter::new_record(RecordType declarations) {
	auto result = m_gc->new_record(std::move(declarations));
	run_gc_if_needed();
	return result;
}

gc_ptr<Function> Interpreter::new_function(FunctionType def, CapturesType s) {
	auto result = m_gc->new_function(def, std::move(s));
	run_gc_if_needed();
	return result;
}

gc_ptr<Error> Interpreter::new_error(std::string e) {
	auto result = m_gc->new_error(e);
	run_gc_if_needed();
	return result;
}

gc_ptr<Variable> Interpreter::new_variable(Value v) {
	assert(v.type() != ValueTag::Variable);
	auto result = m_gc->new_variable(v);
	run_gc_if_needed();
	return result;
}

} // namespace Interpreter
