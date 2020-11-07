#pragma once

namespace TypedAST {
struct TypedAST;
}

namespace Interpreter {

struct Interpreter;

void eval(TypedAST::TypedAST*, Interpreter&);

}
