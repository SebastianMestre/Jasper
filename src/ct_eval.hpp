#pragma once

namespace TypedAST {
struct TypedAST;
struct Allocator;
}

namespace TypeChecker {

struct TypeChecker;

TypedAST::TypedAST* ct_eval(TypedAST::TypedAST*, TypeChecker& tc, TypedAST::Allocator& alloc);

} // namespace TypeChecker
