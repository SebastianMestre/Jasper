#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "algorithms/unification.hpp"
#include "algorithms/union_find.hpp"
#include "meta_unifier.hpp"
#include "typechecker_types.hpp"
#include "utils/interned_string.hpp"

enum class TypeFunctionTag { Builtin, Variant, Record };
// Concrete type function. If it's a built-in, we use argument_count
// to tell how many arguments it takes. Else, for variant, and record,
// we store their structure as a hash from names to monotypes.
//
// Dummy type functions are for unification purposes only, but do not count
// as 'deduced', because they were not created by the user/
//
// TODO: change for polymorphic approach
struct TypeFunctionData {
	TypeFunctionTag tag;
	int argument_count; // -1 means variadic

	std::vector<InternedString> fields;
	std::unordered_map<InternedString, MonoId> structure;

	bool is_dummy {false};
};

// A polytype is a type where some amount of type variables can take
// any value, and still give a valid typing.
struct PolyData {
	MonoId base;
	std::vector<MonoId> vars;
};

struct TypeSystemCore {
	Unification::Core m_mono_core;

	std::vector<TypeFunctionData> m_type_functions;

	std::vector<PolyData> poly_data;

	MetaUnifier m_meta_core;

	TypeSystemCore();

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> args,
	    char const* tag = nullptr);

	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);

	TypeFunctionId new_builtin_type_function(int arguments);
	TypeFunctionId new_type_function(
	    TypeFunctionTag type,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure,
	    bool dummy = false);

	// qualifies all unbound variables in the given monotype
	void gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars);

	MonoId inst_impl(MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);

	TypeFunctionData& type_function_data_of(MonoId);
	TypeFunctionId new_tf_var();
	void unify_tf(TypeFunctionId, TypeFunctionId);
private:
	UnionFind m_tf_uf;

	int compute_new_argument_count(
	    TypeFunctionData const& a_data, TypeFunctionData const& b_data) const;

	void point_tf_at_another(TypeFunctionId a, TypeFunctionId b);
	void unify_tf_data(TypeFunctionData& a_data, TypeFunctionData& b_data);
	TypeFunctionData& get_tf_data(TypeFunctionId tf);
	TypeFunctionId find_tf(TypeFunctionId tf);
};
