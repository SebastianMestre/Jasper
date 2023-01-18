#pragma once

namespace CST {
struct CST;
}

namespace AST {

struct AST;
struct Expr;
struct Allocator;

AST* convert_ast(CST::CST*, Allocator& alloc);
Expr* convert_expr(CST::CST* cst, Allocator& alloc);

}
