#pragma once

#include <vector>
#include <unordered_map>

namespace GarbageCollector {

struct GC;

}

struct ASTFunction;

namespace Type {

struct Value;

using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = std::vector<Value*>;
using FunctionType = ::ASTFunction*;

struct Scope {
	Scope* m_parent {nullptr};
	ObjectType m_declarations;

	Scope() = default;
	Scope(Scope* parent) : m_parent(parent) {}

	Value* access(const Identifier& i) {
		auto v = m_declarations.find(i);
		
		if (v != m_declarations.end())
			return v->second;

		if (m_parent != nullptr)
			return m_parent->access(i);

		// TODO: ReferenceError
		return nullptr;
	}

	void declare(const Identifier& i, Value* v) {
		// TODO: check name colission
		m_declarations[i] = v;
	}
};

struct Environment {
	GarbageCollector::GC* m_gc;
	Scope* m_scope;

	Scope* new_scope() {
		Scope* parent = m_scope;
		m_scope = new Scope(parent);
		return m_scope;
	}

	void end_scope() {
		Scope* parent = m_scope->m_parent;
		delete m_scope;
		m_scope = parent;
	}

	// used as a short-hand
	
	// scope
	inline void declare(const Identifier& i, Value* v) { m_scope->declare(i,v); }
	inline Value* access(const Identifier& i) { return m_scope->access(i); }
	
	// gargabe_collector
	inline Null* null () { return m_gc->null(); }
	inline Integer* new_integer (int i) { return m_gc->new_integer(i); }
	inline Float* new_float (float f) { return m_gc->new_float(f); }
	inline String* new_string (std::string s) { return m_gc->new_string(s); }
	inline List* new_list () { return m_gc->new_list(); }
	inline Object* new_object () { return m_gc->new_object(); }
	inline Function* new_function (FunctionType def, Scope* s) { return m_gc->new_function(def, s); }
	inline Error* new_error (std::string e) { return m_gc->new_error(e); }
};

}
