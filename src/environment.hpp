#pragma once

#include "runtime.hpp"
#include "error.hpp"

namespace GarbageCollector {

struct GC;

}

namespace Type {

struct Scope {
	Scope* m_parent {nullptr};
	ObjectType m_declarations;

	Scope() = default;
	Scope(Scope* parent) : m_parent(parent) {}

	void declare(const Identifier& i, Value* v);
	Value* access(const Identifier& i);
};

struct Environment {
	GarbageCollector::GC* m_gc;
	Scope* m_scope;

	Scope* new_scope();
	void end_scope();

	// used as a short-hand
	
	// scope
	inline void declare(const Identifier&, Value*);
	inline Value* access(const Identifier&);
	
	// gargabe_collector
	inline Null* null();
	inline Integer* new_integer(int);
	inline Float* new_float(float);
	inline String* new_string(std::string);
	inline List* new_list();
	inline Object* new_object();
	inline Function* new_function(FunctionType, Scope*);
	inline Error* new_error(std::string);
};

}
