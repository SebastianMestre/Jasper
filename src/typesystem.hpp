#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include "typechecker_types.hpp"
#include "unification.hpp"

namespace Frontend {
struct CompileTimeEnvironment;
}

enum class TypeFunctionTag { Builtin, Sum, Product, Record };
// Concrete type function. If it's a built-in, we use argument_count
// to tell how many arguments it takes. Else, for sum, product and record,
// we store their structure as a hash from names to monotypes.
//
// Dummy type functions are for unifying purposes only, but do not count
// as 'deduced', because they were not created by the user/
//
// TODO: change for polymorphic approach
struct TypeFunctionData {
	TypeFunctionTag tag;
	int argument_count; // -1 means variadic
	std::unordered_map<std::string, MonoId> structure; // can be nullptr
	bool is_dummy {false};
};

// A polytype is a type where some amount of type variables can take
// any value, and still give a valid typing.
struct PolyData {
	MonoId base;
	std::vector<MonoId> vars;
};

enum class KindTag { TypeFunction, Mono, Poly };
// variable that can contain types, of any kind
struct TypeVarData {
	KindTag kind;
	TypeVarId type_var_id;
};

struct TypeSystemCore {
	// A monotype is a reference to a concrete type. It can be a
	// variable or a term.
	// We express this variant using an enum, and an index that points
	// to where the data is in the correct TypeSystemCore vector.
	//
	// If type is MonoTag::Var, the data_id index points to a
	// different mono in TypeSystemCore::mono_data.
	// It may point to itself, meaning that the type is not known.
	//
	// If the type is MonoTag::Term, the index points to a term,
	// that is stored in TypeSystemCore::term_data
	Unification::Core m_mono_core;

	Unification::Core m_tf_core;
	std::vector<TypeFunctionData> m_type_functions;

	std::vector<PolyData> poly_data;

	std::vector<TypeVarData> type_vars;

	Unification::Core m_meta_core;

	TypeSystemCore();

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> args,
	    char const* tag = nullptr);

	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);

	TypeFunctionId new_builtin_type_function(int arguments);
	TypeFunctionId new_dummy_type_function(
	    TypeFunctionTag type, std::unordered_map<std::string, MonoId> structure);
	
	// NOTE: using int here is provisional
	TypeVarId new_type_var(KindTag kind, int type_id);

	// qualifies all unbound variables in the given monotype
	void gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars);

	MonoId inst_impl(MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);

	void print_type(MonoId, int d = 0);

	// union find for type variables
	// TODO: type safe ids to overload these functions.
	// Maybe even use type vars as the interface for every type-ish thing
	TypeVarData var_find(TypeVarId type_var);
	void var_unify(TypeVarId a, TypeVarId b);
};
