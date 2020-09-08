#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "typesystem_types.hpp"

namespace Frontend {
struct CompileTimeEnvironment;
}

// A type function gives the 'real value' of a type.
// This can refer to a sum type, a product type, a built-in type, etc.
// For tyhe purposes of the type system, we only care about the amount
// of argument it takes.
struct TypeFunctionData {
	// -1 means variadic.
	int argument_count;
};

enum class mono_type { Var, Term };
// A monotype is a reference to a concrete type. It can be a
// variable or a term.
// We express this variant using an enum, and an index that points
// to the where the data is in the correct TypeSystemCore vector.
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

// A variable is just a name for a different monotype.
// VarData stores a MonoID that indicates which monotype it is equal
// to. It can also indicate that it is equal to itself, meaning that
// we don't know its concrete type.
struct VarData {
	MonoId equals;
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

struct TypeSystemCore {
	std::vector<MonoData> mono_data;
	std::vector<TermData> term_data;

	std::vector<TypeFunctionData> type_function_data;
	std::vector<PolyData> poly_data;

	MonoId new_var();
	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> args,
	    char const* tag = nullptr);
	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);

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
};
