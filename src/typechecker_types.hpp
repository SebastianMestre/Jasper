#pragma once

// TODO: make these type-safe
using TypeFunctionId = int;
using TermId = int;
using MonoId = int;
using PolyId = int;
using TypeVarId = int;

using MetaTypeId = int;

namespace TypeChecker {

struct BuiltinType {
	static constexpr TypeFunctionId Function {0};
	static constexpr TypeFunctionId Int {1};
	static constexpr TypeFunctionId Float {2};
	static constexpr TypeFunctionId String {3};
	static constexpr TypeFunctionId Array {4};
	static constexpr TypeFunctionId Boolean {6};
	static constexpr TypeFunctionId Unit {7};
	static constexpr int amount_ {8};
};

} // namespace TypeChecker
