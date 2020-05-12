#pragma once

#include <unordered_set>
#include <string>

struct AST;

std::unordered_set<std::string> gather_captures(AST* ast);
