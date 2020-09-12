#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "typesystem_types.hpp"

namespace Frontend {
struct CompileTimeEnvironment;
}

enum class type_function_type { Var, Known, Sum, Product, Record };
// A type function gives the 'real value' of a type.
// This can refer to a sum type, a product type, a built-in type, etc.
// For the purposes of the type system, we only care about the amount
// of argument it takes.
struct TypeFunctionData {
	int argument_count; // -1 means variadic.
	type_function_type type;
	TypeFunctionId equals; // only for vars
};

enum class mono_type { Var, Term };
// A monotype is a reference to a concrete type. It can be a
// variable or a term.
// We express this variant using an enum, and an index that points
// to where the data is in the correct TypeSystemCore vector.
//
// If type is mono_type::Var, the data_id index points to a
// different mono in TypeSystemCore::mono_data.
// It may point to itself, meaning that the type is not known.
//
// If the type is mono_type::Term, the index points to a term,
// that is stored in TypeSystemCore::term_data
struct MonoData {
	mono_type type;
	int data_id;
};

// A term is an application of a type function.
struct TermData {
	TypeFunctionId type_function;
	std::vector<MonoId> arguments;
	char const* debug_data {nullptr};
};

// A polytype is a type where some amount of type variables can take
// any value, and still give a valid typing.
struct PolyData {
	MonoId base;
	std::vector<MonoId> vars;
};

enum class kind_type { TypeFunction, Mono, Poly };
// variable that can contain types, of any kind
struct TypeVarData {
	kind_type kind;
	TypeVarId type_var_id;
};

struct TypeSystemCore {
	std::vector<MonoData> mono_data;
	std::vector<TermData> term_data;

	std::vector<TypeFunctionData> type_function_data;
	std::vector<PolyData> poly_data;

	std::vector<TypeVarData> type_vars;

	MonoId new_var();
	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> args,
	    char const* tag = nullptr);
	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);
	TypeFunctionId new_type_function(int arguments);
	TypeFunctionId new_type_function_var();
	
	// NOTE: using int here is provisional
	TypeVarId new_type_var(kind_type kind, int type_id);

	// qualifies all unbound variables in the given monotype
	PolyId generalize(MonoId mono, Frontend::CompileTimeEnvironment&);

	void gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars);

	// gives the representative for a given mono
	MonoId find(MonoId mono);

	// var must be a variable that is its own representative
	bool occurs_in(MonoId var, MonoId mono);

	// makes the two given types equal
	void unify(MonoId a, MonoId b);

	MonoId inst_impl(MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);

	void print_type(MonoId, int d = 0);

	// union find for type functions
	// TODO: type safe ids to overload these functions
	TypeVarId func_find(TypeVarId func);
	void func_unify(TypeFunctionId a, TypeFunctionId b);

	// union find for type variables
	// TODO: type safe ids to overload these functions.
	// Maybe even use type vars as the interface for every type-ish thing
	TypeVarData var_find(TypeVarId type_var);
	void var_unify(TypeVarId a, TypeVarId b);
};
