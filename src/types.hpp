#pragma once

#include <vector>
#include <unordered_map>

struct ASTFunction;

namespace Type {

struct Value;

using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = std::vector<Value*>;
using FunctionType = ::ASTFunction*;

}
