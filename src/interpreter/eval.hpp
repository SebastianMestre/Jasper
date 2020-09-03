#pragma once

#include "environment_fwd.hpp"
#include "gc_ptr.hpp"
#include "value_fwd.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace Interpreter {

gc_ptr<Value> eval(TypedAST::TypedAST*, Environment&);

}
