#pragma once

namespace AST {

struct AST;

}

namespace TypeChecker {

void compute_offsets(AST::AST* ast, int frame_offset);

}
