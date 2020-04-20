#pragma once

#include <vector>
#include <unordered_map>

namespace GarbageCollector {

struct GC;

}

struct AST;

namespace Type {

struct Value;

using Identifier = std::string;
using ObjectType = std::unordered_map<Identifier, Value*>;
using ListType = std::vector<Value*>;
using FunctionType = ::AST*;

struct Scope {
	Scope* m_parent;
	ObjectType m_declarations;

	Value* access(Identifier i) {
		auto v = m_declarations.find(i);
		
		if (v != m_declarations.end())
			return v->second;

		if (m_parent != nullptr)
			return m_parent->access(i);

		// TODO: ReferenceError
		return nullptr;
	}
};

struct Environment {
	GarbageCollector::GC* m_gc;
	Scope* m_scope;
};

}
