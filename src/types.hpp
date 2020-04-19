#pragma once

#include <vector>
#include <unordered_map>

struct AST;

namespace Type {

struct Value {
	bool visited = false;
	virtual void gc_visit() = 0;
	virtual ~Value() = default;
};

using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = std::vector<Value*>;
using FunctionType = ::AST*;
using Scope = ObjectType;

}

struct AST {
	virtual void print(int d = 1) = 0;
	virtual Type::Value* run(Type::Scope &s) = 0;
	virtual ~AST() = default;
};
