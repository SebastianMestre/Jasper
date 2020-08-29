#pragma once

#include "value_fwd.hpp"
#include "environment_fwd.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace Interpreter {

Value* eval(TypedAST::TypedAST*, Environment&);

}

