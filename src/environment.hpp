#pragma once

#include "value.hpp"
#include "error.hpp"

namespace GarbageCollector {

struct GC;

}

namespace Type {

struct Scope {
	Scope* m_parent {nullptr};
	Scope* m_prev {nullptr};
	ObjectType m_declarations;

	void declare(const Identifier& i, Value* v);
	Value* access(const Identifier& i);
};

struct Environment {
	GarbageCollector::GC* m_gc;
	Scope* m_scope;
	Scope* m_global_scope;
	Value* m_return_value {nullptr};

	Scope* new_scope();
	Scope* new_nested_scope();
	void end_scope();

	void save_return_value(Type::Value*);
	Value* fetch_return_value();

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
	Array* new_list(ArrayType);
	Object* new_object(ObjectType);
	Dictionary* new_dictionary(ObjectType);
	Function* new_function(FunctionType, ObjectType);
	NativeFunction* new_native_function(NativeFunctionType*);
	Error* new_error(std::string);
};
}
