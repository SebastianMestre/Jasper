#pragma once

#include "value.hpp"
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
	Type::Value* m_return_value {nullptr};

	Scope* new_scope();
	void end_scope();

	void save_return_value(Type::Value*);
	Type::Value* fetch_return_value();

	// used as a short-hand

	// scope
	void declare(const Identifier&, Value*);
	Value* access(const Identifier&);

	// gargabe_collector
	Null* null();
	Integer* new_integer(int);
	Float* new_float(float);
	Boolean* new_boolean(bool);
	String* new_string(std::string);
	List* new_list(std::vector<Type::Value*>);
	Object* new_object();
	Function* new_function(FunctionType, ObjectType const&);
	Error* new_error(std::string);
};
}
