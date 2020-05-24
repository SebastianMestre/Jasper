#pragma once

#include <unordered_set>
#include <string>

struct TypedAST;

std::unordered_set<std::string> gather_captures(TypedAST* ast);
