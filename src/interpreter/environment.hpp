#pragma once

#include "../interned_string.hpp"
#include "error.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

#include <map>

namespace Interpreter {

struct GC;

struct Scope {
	std::map<InternedString, Reference*> m_declarations;

	void declare(const Identifier& i, Reference* v);
	Reference* access(const Identifier& i);
};

struct Environment {
	GC* m_gc;
	Scope m_global_scope;

	Value* m_return_value {nullptr};

	int m_frame_ptr {0};
	int m_stack_ptr {0};
	std::vector<Reference*> m_stack;
	std::vector<int> m_fp_stack;
	std::vector<int> m_sp_stack;

	Environment(GC* gc)
	    : m_gc {gc} {}

	void save_return_value(Value*);
	Value* fetch_return_value();

	void run_gc();

	// Binds a global name to the given reference
	void global_declare(const Identifier& i, Reference* v);
	void global_declare(const Identifier& i, Value* v);
	void global_declare(const Identifier& i, gc_ptr<Value> v);

	Reference* global_access(const Identifier& i);

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
