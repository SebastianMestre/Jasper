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



void Environment::direct_declare(const Identifier& i, Reference* r) {
	if(r->type() != value_type::Reference){
		assert(0 && "directly declared a non-reference!");
	}
	m_scope->declare(i, r);
}

void Environment::declare(const Identifier& i, Value* v) {
	if(v->type() == value_type::Reference){
		assert(0 && "declared a reference!");
	}
	auto r = new_reference(v);
	m_scope->declare(i, r);
}

Reference* Environment::access(const Identifier& i) {
	return m_scope->access(i);
}



Null* Environment::null()
{ return m_gc->null(); }

Integer* Environment::new_integer(int i)
{ return m_gc->new_integer(i); }

Float* Environment::new_float(float f)
{ return m_gc->new_float(f); }

Boolean* Environment::new_boolean(bool b)
{ return m_gc->new_boolean(b); }

String* Environment::new_string(std::string s)
{ return m_gc->new_string(s); }

Array* Environment::new_list(ArrayType elements)
{ return m_gc->new_list(std::move(elements)); }

Object* Environment::new_object(ObjectType declarations)
{ return m_gc->new_object(std::move(declarations)); }

Dictionary* Environment::new_dictionary(ObjectType declarations)
{ return m_gc->new_dictionary(std::move(declarations)); }

Function* Environment::new_function(FunctionType def, ObjectType s)
{ return m_gc->new_function(def, std::move(s)); }

NativeFunction* Environment::new_native_function(NativeFunctionType* fptr)
{ return m_gc->new_native_function(fptr); }

Error* Environment::new_error(std::string e)
{ return m_gc->new_error(e); }

Reference* Environment::new_reference(Value* v) {
	assert(
	    v->type() != value_type::Reference
	    && "References to references are not allowed.");
	return m_gc->new_reference(v);
}

} // Interpreter
