#pragma once

namespace AST {
struct Program;
struct Expr;
struct Allocator;
}

namespace TypeChecker {

struct TypeChecker;

void reify_types(AST::Program*, TypeChecker& tc, AST::Allocator& alloc);

} // namespace TypeChecker
