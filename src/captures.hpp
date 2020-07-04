#pragma once

#include <unordered_set>
#include <string>

namespace TypedAST {
struct TypedAST;
}

std::unordered_set<std::string> gather_captures(TypedAST::TypedAST* ast);
