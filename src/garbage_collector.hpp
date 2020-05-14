#pragma once

#include <vector>

#include "value.hpp"
#include "error.hpp"

namespace GarbageCollector {

struct GC {
private:
	Type::Null* m_null;

public:
	std::vector<Type::Value*> m_blocks;
	std::vector<Type::Value*> m_roots;

	GC();
	~GC();
	
	void run ();
	void add_root (Type::Value* new_root);
	Type::Null* null();

	Type::Object* new_object (Type::ObjectType);
	Type::Dictionary* new_dictionary (Type::ObjectType);
	Type::List* new_list (Type::ListType);
	Type::Integer* new_integer (int);
	Type::Float* new_float (float);
	Type::Boolean* new_boolean (bool);
	Type::String* new_string (std::string);
	Type::Function* new_function (Type::FunctionType, Type::ObjectType);
	Type::NativeFunction* new_native_function (Type::NativeFunctionType*);
	Type::Error* new_error (std::string);
};

}
