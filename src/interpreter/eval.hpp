#pragma once

namespace TypedAST {
struct TypedAST;
}

namespace Interpreter {

struct Environment;

void eval(TypedAST::TypedAST*, Environment&);

}
