#pragma once

#include <vector>

#include "value.hpp"
#include "error.hpp"

namespace Interpreter {

struct GC {
private:
	Null* m_null;

public:
	std::vector<Value*> m_blocks;
	std::vector<Value*> m_roots;

	GC();
	~GC();
	
	void unmark_all();
	void mark_roots();
	void sweep();
	void sweep_all();

	void add_root(Value* new_root);

	auto null() -> Null*;
	auto new_object(ObjectType) -> Object*;
	auto new_dictionary(ObjectType) -> Dictionary*;
	auto new_list(ArrayType) -> Array*;
	auto new_integer(int) -> Integer*;
	auto new_float(float) -> Float*;
	auto new_boolean(bool) -> Boolean*;
	auto new_string(std::string) -> String*;
	auto new_function(FunctionType, ObjectType) -> Function*;
	auto new_native_function(NativeFunctionType*) -> NativeFunction*;
	auto new_error(std::string) -> Error*;
	auto new_reference(Value*) -> Reference*;
};

} // namespace Interpreter
