#pragma once

namespace TypedAST {

struct TypedAST;

}

namespace TypeChecker {

void compute_offsets(TypedAST::TypedAST* ast, int frame_offset);

}
