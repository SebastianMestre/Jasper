#pragma once

namespace AST {

struct AST;
struct Program;

}

namespace TypeChecker {

void compute_offsets(AST::AST* ast, int frame_offset);
void compute_offsets_program(AST::Program* ast, int frame_offset);

}
