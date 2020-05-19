#pragma once

#include "environment.hpp"
#include "value.hpp"

Type::Value* print(Type::ArrayType v, Type::Environment& e);

Type::Value* array_append(Type::ArrayType v, Type::Environment& e);

Type::Value* array_extend(Type::ArrayType v, Type::Environment& e);

void declare_native_functions(Type::Environment& env);
