#pragma once

#include "environment_fwd.hpp"
#include "gc_ptr.hpp"
#include "value_fwd.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace Interpreter {

void eval(TypedAST::TypedAST*, Environment&);

}
