#pragma once

#include "value_fwd.hpp"
#include "environment_fwd.hpp"
#include "gc_ptr.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace Interpreter {

gc_ptr<Value> eval(TypedAST::TypedAST*, Environment&);

}

