#pragma once

#include "environment.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

#include <map>

namespace TypeChecker {
struct TypeChecker;
}

namespace Interpreter {

struct GC;
struct Error;

struct Scope {
	std::map<InternedString, Reference*> m_declarations;

	void declare(const Identifier& i, Reference* v);
	Reference* access(const Identifier& i);
};

struct Interpreter {
	Environment m_env;
	TypeChecker::TypeChecker* m_tc;
	GC* m_gc;
	int m_gc_size_on_last_pass {64};
	Value* m_return_value {nullptr};
	Scope m_global_scope;

	Interpreter(TypeChecker::TypeChecker* tc, GC* gc)
	    : m_tc {tc}
	    , m_gc {gc} {}

	void save_return_value(Value*);
	Value* fetch_return_value();

	void run_gc();
	void run_gc_if_needed();

	// Binds a global name to the given reference
	void global_declare_direct(const Identifier& i, Reference* v);
	void global_declare(const Identifier& i, Value* v);
	void global_declare(const Identifier& i, gc_ptr<Value> v);
	Reference* global_access(const Identifier& i);


	auto null() -> Null*;
	void push_integer(int);
	void push_float(float);
	void push_boolean(bool);
	void push_string(std::string);
	void push_struct_constructor(std::vector<InternedString>);
	auto new_list(ArrayType) -> gc_ptr<Array>;
	auto new_object(ObjectType) -> gc_ptr<Object>;
	auto new_dictionary(DictionaryType) -> gc_ptr<Dictionary>;
	auto new_function(FunctionType, ObjectType) -> gc_ptr<Function>;
	auto new_native_function(NativeFunctionType*) -> gc_ptr<NativeFunction>;
	auto new_error(std::string) -> gc_ptr<Error>;
	auto new_reference(Value*) -> gc_ptr<Reference>;
};

} // namespace Interpreter