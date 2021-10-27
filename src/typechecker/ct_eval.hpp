#pragma once

namespace AST {
struct AST;
struct Expr;
struct Allocator;
}

namespace TypeChecker {

struct TypeChecker;

void reify_types(AST::AST*, TypeChecker& tc, AST::Allocator& alloc);

} // namespace TypeChecker
