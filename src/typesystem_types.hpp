#pragma once

// TODO: make these type-safe
using TypeFunctionId = int;
using TermId = int;
using MonoId = int;
using PolyId = int;
using TypeVarId = int;

namespace TypeChecker {

struct BuiltinType {
	static constexpr TypeFunctionId Function {0};
	static constexpr TypeFunctionId Int {1};
	static constexpr TypeFunctionId Float {2};
	static constexpr TypeFunctionId String {3};
	static constexpr TypeFunctionId Array {4};
	static constexpr TypeFunctionId Dictionary {5};
	static constexpr TypeFunctionId Boolean {6};
	static constexpr TypeFunctionId Unit {7};
	static constexpr TypeFunctionId Sum {8};
	static constexpr TypeFunctionId Product {9};
	static constexpr TypeFunctionId Record {10};
	static constexpr int amount_ {11};
};

} // namespace TypeChecker
