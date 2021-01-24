#pragma once

namespace AST {
struct AST;
}

namespace TypeChecker {

struct TypeChecker;

void metacheck(AST::AST* ast, TypeChecker& tc);

}
