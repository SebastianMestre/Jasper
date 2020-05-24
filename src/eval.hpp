#pragma once

#include "value_fwd.hpp"
#include "environment_fwd.hpp"

struct TypedAST;

Type::Value* eval(TypedAST*, Type::Environment&);

