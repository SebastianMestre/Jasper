#pragma once

namespace AST {
struct Expr;
struct Program;
}

namespace Interpreter {

struct Interpreter;

void run(AST::Program*, Interpreter&);
void eval(AST::Expr*, Interpreter&);

}
