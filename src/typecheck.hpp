#pragma once

namespace TypedAST {
struct TypedAST;
}

namespace Frontend {
struct CompileTimeEnvironment;
}

namespace TypeChecker {

/*
 * Runs type deduction and checking on the given ast.
 *
 * Implements a variant of the Hindley-Milbner type
 * inference algorithm.
 *
 * PRECONDITION: match_identifiers has already been called
 * the given ast.
 */
void typecheck(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment&);
} // namespace TypeChecker
