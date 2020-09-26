#pragma once

namespace TypedAST {
struct TypedAST;
}

namespace TypeChecker {

struct TypeChecker;

void metacheck(TypedAST::TypedAST* ast, TypeChecker& tc);

}
