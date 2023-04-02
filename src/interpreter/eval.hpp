#pragma once

namespace AST {
struct AST;
struct Program;
}

namespace Interpreter {

struct Interpreter;

void run(AST::Program*, Interpreter&);
void eval(AST::AST*, Interpreter&);

}
