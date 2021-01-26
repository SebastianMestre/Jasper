#pragma once

namespace AST {
struct AST;
}

namespace Interpreter {

struct Interpreter;

void eval(AST::AST*, Interpreter&);

}
