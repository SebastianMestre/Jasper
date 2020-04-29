#include "environment.hpp"
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

// used as a short-hand

// scope
inline void Environment::declare(const Identifier& i, Value* v) { m_scope->declare(i,v); }
inline Value* Environment::access(const Identifier& i) { return m_scope->access(i); }

// gargabe_collector
inline Null* Environment::null() { return m_gc->null(); }
inline Integer* Environment::new_integer(int i) { return m_gc->new_integer(i); }
inline Float* Environment::new_float(float f) { return m_gc->new_float(f); }
inline String* Environment::new_string(std::string s) { return m_gc->new_string(s); }
inline List* Environment::new_list() { return m_gc->new_list(); }
inline Object* Environment::new_object() { return m_gc->new_object(); }
inline Function* Environment::new_function(FunctionType def, Scope* s) { return m_gc->new_function(def, s); }
inline Error* Environment::new_error(std::string e) { return m_gc->new_error(e); }

}
