#pragma once

namespace AST {
struct Expr;
struct AST;
struct Allocator;
}

namespace TypeChecker {

struct TypeChecker;

void rewrite_after_metacheck(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc);

}
