#pragma once

namespace CST {
struct CST;
struct Program;
struct Expr;
}

namespace AST {

struct AST;
struct Expr;
struct Program;
struct Allocator;

Program* convert_program(CST::Program*, Allocator& alloc);

Expr* convert_expr(CST::Expr* cst, Allocator& alloc);

}
