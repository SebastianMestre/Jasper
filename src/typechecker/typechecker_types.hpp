#pragma once

// TODO: make these type-safe
using TermId = int;
using MonoId = int;
using PolyId = int;

enum class TypeFunc {};

enum class MetaType { Term, Type, TypeFunction, Constructor, Undefined };

namespace TypeChecker {

struct BuiltinType {
	static constexpr TypeFunc Function = TypeFunc(0);
	static constexpr TypeFunc Int = TypeFunc(1);
	static constexpr TypeFunc Float = TypeFunc(2);
	static constexpr TypeFunc String = TypeFunc(3);
	static constexpr TypeFunc Array = TypeFunc(4);
	static constexpr TypeFunc Boolean = TypeFunc(5);
	static constexpr TypeFunc Unit = TypeFunc(6);
	static constexpr int amount_ {7};
};

} // namespace TypeChecker
