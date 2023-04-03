#pragma once

namespace AST {

struct Expr;
struct Program;

}

namespace TypeChecker {

void compute_offsets(AST::Expr* ast, int frame_offset);
void compute_offsets_program(AST::Program* ast, int frame_offset);

}
