#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../algorithms/union_find.hpp"
#include "../log/log.hpp"
#include "../utils/interned_string.hpp"
#include "meta_unifier.hpp"
#include "typechecker_types.hpp"

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
	enum class Tag { Var, App };
	Tag tag;

	// data for app case
	int func_id;
	std::vector<int> args;

	// constraints
	TypeData data;
};

struct TypeSystemCore {

	std::vector<MonoFr> m_monos;
	Uf m_monos_uf;

	std::vector<TypeFunctionData> m_type_functions;
	Uf m_type_function_uf;

	std::vector<PolyData> poly_data;

	MetaUnifier m_meta_core;

	TypeSystemCore() = default;

	MonoId new_term(TypeFunctionId type_function, std::vector<MonoId> args);

	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);

	void unify_type_function(TypeFunctionId a, TypeFunctionId b);
	TypeFunctionId new_builtin_type_function(int arguments);
	TypeFunctionId new_type_function(
	    TypeFunctionTag type,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure,
	    bool dummy = false);
	TypeFunctionId new_type_function_var();

	MonoId new_constrained_term(
	    TypeFunctionTag type, std::unordered_map<InternedString, MonoId> structure);

	// qualifies all unbound variables in the given monotype
	void gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars);
	MonoId inst_impl(MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);
	MonoId new_var() { return new_constrained_term(TypeFunctionTag::Any, {}); }

	void combine_left_to_right(MonoId, MonoId);
	void combine_left_to_right(TypeData&, TypeData&);
	void check_constraints_left_to_right(TypeData&, TypeData&);

	void unify(MonoId lhs, MonoId rhs);
	int find_function(MonoId x);
	bool occurs(int i, int j);
};
