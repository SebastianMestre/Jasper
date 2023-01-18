#pragma once

namespace CST {
struct CST;
struct Program;
}

namespace AST {

struct AST;
struct Expr;
struct Program;
struct Allocator;

AST* convert_ast(CST::CST*, Allocator& alloc);
Program* convert_program(CST::Program*, Allocator& alloc);
Expr* convert_expr(CST::CST* cst, Allocator& alloc);

}
