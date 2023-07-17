#pragma once

namespace AST {
struct Expr;
struct Program;
}

namespace TypeChecker {

struct TypeChecker;

/*
 * Runs type deduction and checking on the given ast.
 *
 * Implements a variant of the Hindley-Milner type
 * inference algorithm.
 *
 * PRECONDITION: match_identifiers has already been called
 * the given ast.
 */
// void typecheck(AST::Expr* ast, TypeChecker&);
void typecheck_program(AST::Program* ast, TypeChecker&);
} // namespace TypeChecker
