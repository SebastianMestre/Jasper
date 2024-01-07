#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../algorithms/union_find.hpp"
#include "../utils/interned_string.hpp"
#include "typechecker_types.hpp"

enum class VarId {};

inline bool operator==(VarId a, VarId b) {
	return static_cast<int>(a) == static_cast<int>(b);
}

template<>
struct std::hash<VarId> {
	size_t operator()(VarId v) const {
		return std::hash<int>{}(static_cast<int>(v));
	}
};



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
};

// A polytype is a type where some amount of type variables can take
// any value, and still give a valid typing.
struct PolyData {
	MonoId base;
	std::vector<VarId> vars;
};

struct Constraint {
	enum class Shape {
		Unknown, Variant, Record
	};

	std::unordered_map<InternedString, MonoId> structure;
	Shape shape;
};

struct TypeSystemCore {

	TypeSystemCore();

	// types

	std::unordered_set<VarId> free_vars(MonoId);
	void ll_unify(MonoId i, MonoId j);
	TypeFunctionData& type_function_data_of(MonoId);
	VarId get_var_id(MonoId i);

	MonoId ll_new_var();
	MonoId new_term(TypeFunctionId type_function, std::vector<MonoId> args);

	MonoId fun(std::vector<MonoId> arg_tys, MonoId res_ty) {
		arg_tys.push_back(res_ty);
		return new_term(TypeChecker::BuiltinType::Function, {arg_tys});
	}

	MonoId array(MonoId elem_ty) {
		return new_term(TypeChecker::BuiltinType::Array, {elem_ty});
	}

	// typevars

	void add_record_constraint(VarId);
	void add_variant_constraint(VarId);
	void add_field_constraint(VarId, InternedString name, MonoId ty);

	// polytypes

	MonoId inst_fresh(PolyId poly);
	PolyId forall(std::vector<VarId>, MonoId);

	// typefuncs

	TypeFunctionData& get_type_function_data(TypeFunctionId);
	TypeFunctionId new_builtin_type_function(int arguments);

	TypeFunctionId new_record(std::vector<InternedString> fields, std::vector<MonoId> const& types) {
		std::unordered_map<InternedString, MonoId> structure;
		for (int i = 0; i < fields.size(); ++i)
			structure[fields[i]] = types[i];
		return new_type_function(
		    TypeFunctionTag::Record, std::move(fields), std::move(structure));
	}

	TypeFunctionId new_variant(std::vector<InternedString> const& fields, std::vector<MonoId> const& types) {
		std::unordered_map<InternedString, MonoId> structure;
		for (int i = 0; i < fields.size(); ++i)
			structure[fields[i]] = types[i];
		return new_type_function(TypeFunctionTag::Variant, {}, std::move(structure));
	}

private:

	TypeFunctionId new_type_function(
	    TypeFunctionTag type,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure);
	void gather_free_vars(MonoId, std::unordered_set<VarId>&);

	MonoId inst_impl(MonoId mono, std::unordered_map<VarId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	void unify_type_function(TypeFunctionId, TypeFunctionId);

	enum class Tag { Var, Term, };

	struct NodeHeader {
		Tag tag;
		int data_idx;
	};

	struct TermData {
		int function_id; // external id
		std::vector<int> argument_idx;
	};

	int ll_find(int i);
	int ll_find_term(int i);
	int ll_find_function(int i);

	bool ll_is_var(int i);
	bool ll_is_term(int i);

	int ll_new_term(int f, std::vector<int> args = {});

	TypeFunctionId create_type_function(
	    TypeFunctionTag tag,
	    int arity,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure);

	void establish_substitution(VarId var_id, int type_id);

	bool occurs(VarId v, MonoId i);
	bool equals_var(MonoId t, VarId v);
	void unify_vars_left_to_right(VarId vi, VarId vj);
	void combine_constraints_left_to_right(VarId vi, VarId vj);
	bool satisfies(MonoId t, Constraint const& c);
	// per-func data
	std::vector<TypeFunctionData> m_type_functions;

	// per-poly data
	std::vector<PolyData> poly_data;

	// per-type data
	std::vector<NodeHeader> ll_node_header;

	// per-term data
	std::vector<TermData> ll_term_data;

	// per-var data
	std::vector<MonoId> m_substitution;
	std::vector<Constraint> m_constraints;
	UnionFind m_type_var_uf;

	int m_var_counter {0};
	int m_term_counter {0};
	int m_type_counter {0};
};
