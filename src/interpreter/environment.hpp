#pragma once

#include "../interned_string.hpp"
#include "error.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct GC;

struct Scope {
	Scope* m_parent {nullptr};
	Scope* m_prev {nullptr};
	// TODO: store references instead of values
	ObjectType m_declarations;

	void declare(const Identifier& i, Reference* v);
	Reference* access(const Identifier& i);
};

struct Environment {
	GC* m_gc;
	Scope m_global_scope;

	Scope* m_scope;
	Value* m_return_value {nullptr};

	Environment(GC* gc)
	    : m_gc {gc}
	    , m_scope {&m_global_scope} {}

	Scope* new_scope();
	Scope* new_nested_scope();
	void end_scope();

	void save_return_value(Value*);
	Value* fetch_return_value();

	void run_gc();

	// SHORT-HANDS

	// Binds a name to a new reference of the given value
	void declare(const Identifier&, Value*);
	void declare(const Identifier&, gc_ptr<Value>);
	// Binds a name to the given reference
	void direct_declare(const Identifier& i, Reference* v);
	Reference* access(const Identifier&);

	auto null() -> Null*;
	auto new_integer(int) -> gc_ptr<Integer>;
	auto new_float(float) -> gc_ptr<Float>;
	auto new_boolean(bool) -> gc_ptr<Boolean>;
	auto new_string(std::string) -> gc_ptr<String>;
	auto new_list(ArrayType) -> gc_ptr<Array>;
	auto new_object(ObjectType) -> gc_ptr<Object>;
	auto new_dictionary(DictionaryType) -> gc_ptr<Dictionary>;
	auto new_function(FunctionType, ObjectType) -> gc_ptr<Function>;
	auto new_native_function(NativeFunctionType*) -> gc_ptr<NativeFunction>;
	auto new_error(std::string) -> gc_ptr<Error>;
	auto new_reference(Value*) -> gc_ptr<Reference>;
};

} // namespace Interpreter
