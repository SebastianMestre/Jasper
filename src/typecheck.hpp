#pragma once

namespace TypedAST {
struct TypedAST;
}

namespace Frontend {
struct CompileTimeEnvironment;
}

namespace TypeChecker {

void typecheck(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment&);

}
