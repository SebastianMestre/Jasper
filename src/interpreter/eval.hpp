#pragma once

#include "value_fwd.hpp"
#include "environment_fwd.hpp"

namespace TypedAST {
struct TypedAST;
}

Type::Value* eval(TypedAST::TypedAST*, Type::Environment&);

