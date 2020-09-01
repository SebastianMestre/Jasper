#include "environment.hpp"

#include <cassert>

#include "garbage_collector.hpp"

namespace Interpreter {

void Scope::declare(const Identifier& i, Reference* v) {
	// TODO: check name collision
	m_declarations[i] = v;
}

Reference* Scope::access(const Identifier& i) {
	auto v = m_declarations.find(i);

	if (v != m_declarations.end()){
		assert(v->second->type() == value_type::Reference);
		return static_cast<Reference*>(v->second);
	}

	if (m_parent != nullptr)
		return m_parent->access(i);

	// TODO: ReferenceError
	return nullptr;
}



Scope* Environment::new_nested_scope() {
	m_scope = new Scope{m_scope, m_scope};
	return m_scope;
}

Scope* Environment::new_scope() {
	m_scope = new Scope{&m_global_scope, m_scope};
	return m_scope;
}

void Environment::end_scope() {
	Scope* prev = m_scope->m_prev;
	delete m_scope;
	m_scope = prev;
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



void Environment::direct_declare(const Identifier& i, Reference* r) {
	if(r->type() != value_type::Reference){
		assert(0 && "directly declared a non-reference!");
	}
	m_scope->declare(i, r);
}

void Environment::declare(const Identifier& i, gc_ptr<Value> v) {
	declare(i, v.get());
}

void Environment::declare(const Identifier& i, Value* v) {
	if(v->type() == value_type::Reference){
		assert(0 && "declared a reference!");
	}
	auto r = new_reference(v);
	m_scope->declare(i, r.get());
}

Reference* Environment::access(const Identifier& i) {
	return m_scope->access(i);
}



Null* Environment::null()
{ return m_gc->null(); }

gc_ptr<Integer> Environment::new_integer(int i)
{ return m_gc->new_integer(i); }

gc_ptr<Float> Environment::new_float(float f)
{ return m_gc->new_float(f); }

gc_ptr<Boolean> Environment::new_boolean(bool b)
{ return m_gc->new_boolean(b); }

gc_ptr<String> Environment::new_string(std::string s)
{ return m_gc->new_string(std::move(s)); }

gc_ptr<Array> Environment::new_list(ArrayType elements)
{ return m_gc->new_list_unsafe(std::move(elements)); }

gc_ptr<Object> Environment::new_object(ObjectType declarations)
{ return m_gc->new_object_unsafe(std::move(declarations)); }

gc_ptr<Dictionary> Environment::new_dictionary(ObjectType declarations)
{ return m_gc->new_dictionary_unsafe(std::move(declarations)); }

gc_ptr<Function> Environment::new_function(FunctionType def, ObjectType s)
{ return m_gc->new_function_unsafe(def, std::move(s)); }

gc_ptr<NativeFunction> Environment::new_native_function(NativeFunctionType* fptr)
{ return m_gc->new_native_function_unsafe(fptr); }

gc_ptr<Error> Environment::new_error(std::string e)
{ return m_gc->new_error_unsafe(e); }

gc_ptr<Reference> Environment::new_reference(Value* v) {
	assert(
	    v->type() != value_type::Reference
	    && "References to references are not allowed.");
	return m_gc->new_reference_unsafe(v);
}

} // Interpreter
