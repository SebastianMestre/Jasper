#pragma once

namespace TypedAST {
struct TypedAST;
}

namespace TypeChecker {

struct TypeChecker;

/*
 * Runs type deduction and checking on the given ast.
 *
 * Implements a variant of the Hindley-Milbner type
 * inference algorithm.
 *
 * PRECONDITION: match_identifiers has already been called
 * the given ast.
 */
void typecheck(TypedAST::TypedAST* ast, TypeChecker&);
} // namespace TypeChecker
