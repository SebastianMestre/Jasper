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

	if (v != m_declarations.end()) {
		assert(v->second->type() == ValueTag::Reference);
		return static_cast<Reference*>(v->second);
	}

	if (m_parent != nullptr)
		return m_parent->access(i);

	// TODO: ReferenceError
	return nullptr;
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

void Environment::run_gc() {
	m_gc->unmark_all();
	m_gc->mark_roots();

	for (Scope* scope_it = m_scope; scope_it != nullptr; scope_it = scope_it->m_prev)
		for (auto& p : scope_it->m_declarations)
			gc_visit(p.second);

	m_gc->sweep();
}

void Environment::global_declare(const Identifier& i, Reference* r) {
	m_global_scope.declare(i, r);
}

void Environment::global_declare(const Identifier& i, Value* v) {
	if (v->type() == ValueTag::Reference)
		assert(0 && "declared a reference!");
	auto r = new_reference(v);
	global_declare(i, r.get());
}

void Environment::global_declare(const Identifier& i, gc_ptr<Value> v) {
	global_declare(i, v.get());
}

Reference* Environment::global_access(const Identifier& i) {
	return m_global_scope.access(i);
}

Null* Environment::null() {
	return m_gc->null();
}

gc_ptr<Integer> Environment::new_integer(int i) {
	return m_gc->new_integer(i);
}

gc_ptr<Float> Environment::new_float(float f) {
	return m_gc->new_float(f);
}

gc_ptr<Boolean> Environment::new_boolean(bool b) {
	return m_gc->new_boolean(b);
}

gc_ptr<String> Environment::new_string(std::string s) {
	return m_gc->new_string(std::move(s));
}

gc_ptr<Array> Environment::new_list(ArrayType elements) {
	return m_gc->new_list(std::move(elements));
}

gc_ptr<Object> Environment::new_object(ObjectType declarations) {
	return m_gc->new_object(std::move(declarations));
}

gc_ptr<Dictionary> Environment::new_dictionary(DictionaryType declarations) {
	return m_gc->new_dictionary(std::move(declarations));
}

gc_ptr<Function> Environment::new_function(FunctionType def, ObjectType s) {
	return m_gc->new_function(def, std::move(s));
}

gc_ptr<NativeFunction> Environment::new_native_function(NativeFunctionType* fptr) {
	return m_gc->new_native_function(fptr);
}

gc_ptr<Error> Environment::new_error(std::string e) {
	return m_gc->new_error(e);
}

gc_ptr<Reference> Environment::new_reference(Value* v) {
	assert(
	    v->type() != ValueTag::Reference &&
	    "References to references are not allowed.");
	return m_gc->new_reference(v);
}

} // namespace Interpreter
