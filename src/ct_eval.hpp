#pragma once

namespace AST {
struct AST;
struct Allocator;
}

namespace TypeChecker {

struct TypeChecker;

AST::AST* ct_eval(AST::AST*, TypeChecker& tc, AST::Allocator& alloc);

} // namespace TypeChecker
