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
	void add_root(Value* new_root);
	Null* null();

	Object*         new_object(ObjectType);
	Dictionary*     new_dictionary(ObjectType);
	Array*          new_list(ArrayType);
	Integer*        new_integer(int);
	Float*          new_float(float);
	Boolean*        new_boolean(bool);
	String*         new_string(std::string);
	Function*       new_function(FunctionType, ObjectType);
	NativeFunction* new_native_function(NativeFunctionType*);
	Error*          new_error(std::string);
	Reference*      new_reference(Value*);
};

} // namespace Interpreter
