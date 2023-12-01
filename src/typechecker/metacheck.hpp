#pragma once

namespace AST {
struct Expr;
struct Program;
}

struct MetaUnifier;

namespace TypeChecker {

// void metacheck(MetaUnifier&, AST::Expr*);
void metacheck_program(AST::Program* ast);

}
