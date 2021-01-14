#pragma once

#include <vector>

namespace TypedAST {
struct TypedAST;
}

struct Instruction;

std::vector<Instruction> emit_bytecode(TypedAST::TypedAST* ast);
