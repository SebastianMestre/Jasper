#pragma once

namespace AST {
struct AST;
struct Program;
}

struct MetaUnifier;

namespace TypeChecker {

void metacheck(MetaUnifier&, AST::AST*);
void metacheck_program(MetaUnifier& uf, AST::Program* ast);

}
