#include "environment.hpp"

#include <cassert>

#include "garbage_collector.hpp"

namespace Type {

void Scope::declare(const Identifier& i, Value* v) {
	// TODO: check name colission
	m_declarations[i] = v;
}

Value* Scope::access(const Identifier& i) {
	auto v = m_declarations.find(i);

	if (v != m_declarations.end())
		return v->second;

	if (m_parent != nullptr)
		return m_parent->access(i);

	// TODO: ReferenceError
	return nullptr;
}



Scope* Environment::new_scope() {
	Scope* parent = m_scope;
	m_scope = new Scope(parent);
	return m_scope;
}

void Environment::end_scope() {
	Scope* parent = m_scope->m_parent;
	delete m_scope;
	m_scope = parent;
}

void Environment::save_return_value(Type::Value* v) {
	// check if not stepping on another value
	assert(!m_return_value);
	m_return_value = v;
}

Value* Environment::fetch_return_value() {
	Value* rv = m_return_value;
	m_return_value = nullptr;
	return rv;
}

// used as a short-hand

// scope
void Environment::declare(const Identifier& i, Value* v) { m_scope->declare(i,v); }
Value* Environment::access(const Identifier& i) { return m_scope->access(i); }

// garbage_collector
Null* Environment::null() { return m_gc->null(); }
Integer* Environment::new_integer(int i) { return m_gc->new_integer(i); }
Float* Environment::new_float(float f) { return m_gc->new_float(f); }
Boolean* Environment::new_boolean(bool b) { return m_gc->new_boolean(b); }
String* Environment::new_string(std::string s) { return m_gc->new_string(s); }
Array* Environment::new_list(ArrayType elements) { return m_gc->new_list(std::move(elements)); }
Object* Environment::new_object(ObjectType declarations) { return m_gc->new_object(std::move(declarations)); }
Dictionary* Environment::new_dictionary(ObjectType declarations) { return m_gc->new_dictionary(std::move(declarations)); }
Function* Environment::new_function(FunctionType def, ObjectType s) { return m_gc->new_function(def, std::move(s)); }
NativeFunction* Environment::new_native_function(NativeFunctionType* fptr) { return m_gc->new_native_function(fptr); }
Error* Environment::new_error(std::string e) { return m_gc->new_error(e); }
}
