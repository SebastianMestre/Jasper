#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "algorithms/unification.hpp"
#include "meta_unifier.hpp"
#include "typechecker_types.hpp"
#include "utils/interned_string.hpp"
#include "uf.hpp"
#include "log/log.hpp"

// Concrete type.
// Else, for variant and record,
// we store their structure as a hash from names to monotypes.
//
// Dummy type functions are for unification purposes only, but do not count
// as 'deduced', because they were not created by the user/
//
// TODO: change for polymorphic approach
enum class TypeFunctionTag { Any, Builtin, Variant, Record };
struct TypeData {
	TypeFunctionTag tag;
	std::vector<InternedString> fields;
	std::unordered_map<InternedString, MonoId> structure;
};

struct MonoDataFr {
	TypeData details;
	bool is_dummy {false};
};

struct TypeFunctionData {
	TypeData result_data;
	int argument_count; // -1 means variadic
	bool is_dummy {false};
};

// A polytype is a type where some amount of type variables can take
// any value, and still give a valid typing.
struct PolyData {
	MonoId base;
	std::vector<MonoId> vars;
};

struct MonoFr {
	enum class Tag { Constr, App };
	Tag tag;

	// app
	int func_id;
	std::vector<int> args;

	// constr
	TypeData data;
};

struct TypeSystemCore {

	std::vector<MonoFr> m_monos;
	Uf m_monos_uf;

	Unification::Core m_tf_core;
	std::vector<TypeFunctionData> m_type_functions;

	std::vector<PolyData> poly_data;

	MetaUnifier m_meta_core;

	TypeSystemCore();

	MonoId new_term(TypeFunctionId type_function, std::vector<MonoId> args);

	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);

	TypeFunctionId new_builtin_type_function(int arguments);
	TypeFunctionId new_type_function(
	    TypeFunctionTag type,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure,
	    bool dummy = false);

	MonoId new_constrained_term(
	    TypeFunctionTag type, std::unordered_map<InternedString, MonoId> structure);

	// qualifies all unbound variables in the given monotype
	void gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars);
	MonoId inst_impl(MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);
	MonoId new_var() { return new_constrained_term(TypeFunctionTag::Any, {}); }
	void combine_left_to_right(MonoId lhs, MonoId rhs);
	void unify(MonoId lhs, MonoId rhs);
	int find_function(MonoId x);
	bool occurs(int i, int j);
};
