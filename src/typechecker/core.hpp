#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../algorithms/union_find.hpp"
#include "../utils/interned_string.hpp"
#include "typechecker_types.hpp"

enum class VarId {};

template<>
struct std::hash<VarId> {
	size_t operator()(VarId v) const {
		return std::hash<int>{}(static_cast<int>(v));
	}
};




struct TypeSystemCore {

	TypeSystemCore();

	// types

	std::unordered_set<VarId> free_vars(Type);
	void ll_unify(Type i, Type j);
	TypeFunc type_function_of(Type);
	VarId get_var_id(Type i);

	Type ll_new_var();
	Type new_term(TypeFunc type_function, std::vector<Type> args);

	Type fun(std::vector<Type> arg_tys, Type res_ty) {
		arg_tys.push_back(res_ty);
		return new_term(TypeChecker::BuiltinType::Function, {arg_tys});
	}

	Type array(Type elem_ty) {
		return new_term(TypeChecker::BuiltinType::Array, {elem_ty});
	}

	// typevars

	void add_record_constraint(VarId);
	void add_variant_constraint(VarId);
	void add_field_constraint(VarId, InternedString name, Type ty);

	// polytypes

	Type inst_fresh(PolyId poly);
	PolyId forall(std::vector<VarId>, Type);

	// typefuncs

	bool is_record(TypeFunc);
	bool is_variant(TypeFunc);
	std::vector<InternedString> const& fields(TypeFunc);
	Type type_of_field(TypeFunc, InternedString);

	TypeFunc new_builtin_type_function(int arguments);

	TypeFunc new_record(std::vector<InternedString> fields, std::vector<Type> const& types) {
		std::unordered_map<InternedString, Type> structure;
		for (int i = 0; i < fields.size(); ++i)
			structure[fields[i]] = types[i];
		return new_type_function(
		    TypeFunctionTag::Record, std::move(fields), std::move(structure));
	}

	TypeFunc new_variant(std::vector<InternedString> const& fields, std::vector<Type> const& types) {
		std::unordered_map<InternedString, Type> structure;
		for (int i = 0; i < fields.size(); ++i)
			structure[fields[i]] = types[i];
		return new_type_function(TypeFunctionTag::Variant, {}, std::move(structure));
	}

private:

	enum class TypeFunctionTag { Builtin, Variant, Record };
	// Concrete type function. If it's a built-in, we use argument_count
	// to tell how many arguments it takes. Else, for variant, and record,
	// we store their structure as a hash from names to monotypes.
	struct TypeFunctionData {
		TypeFunctionTag tag;
		int argument_count; // -1 means variadic

		std::vector<InternedString> fields;
		std::unordered_map<InternedString, Type> structure;
	};

	// A polytype is a type where some amount of type variables can take
	// any value, and still give a valid typing.
	struct PolyData {
		Type base;
		std::vector<VarId> vars;
	};

	struct Constraint {
		enum class Shape {
			Unknown, Variant, Record
		};

		std::unordered_map<InternedString, Type> structure;
		Shape shape;
	};

	enum class Tag { Var, Term, };

	struct NodeHeader {
		Tag tag;
		int data_idx;
	};

	struct TermData {
		TypeFunc function_id; // external id
		std::vector<Type> argument_idx;
	};

	NodeHeader& data(Type);
	TypeFunctionData& data(TypeFunc);

	TypeFunc new_type_function(
	    TypeFunctionTag type,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, Type> structure);
	void gather_free_vars(Type, std::unordered_set<VarId>&);

	Type inst_impl(Type mono, std::unordered_map<VarId, Type> const& mapping);
	Type inst_with(PolyId poly, std::vector<Type> const& vals);
	void unify_type_function(TypeFunc, TypeFunc);

	Type ll_find(Type i);

	bool ll_is_var(Type i);
	bool ll_is_term(Type i);

	Type ll_new_term(TypeFunc f, std::vector<Type> args);

	TypeFunc create_type_function(
	    TypeFunctionTag tag,
		int arity,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, Type> structure);

	void establish_substitution(VarId var_id, Type type_id);

	bool occurs(VarId v, Type i);
	bool equals_var(Type t, VarId v);
	void unify_vars_left_to_right(VarId vi, VarId vj);
	void combine_constraints_left_to_right(VarId vi, VarId vj);
	bool satisfies(Type t, Constraint const& c);
	// per-func data
	std::vector<TypeFunctionData> m_type_functions;

	// per-poly data
	std::vector<PolyData> poly_data;

	// per-type data
	std::vector<NodeHeader> ll_node_header;

	// per-term data
	std::vector<TermData> ll_term_data;

	// per-var data
	std::vector<Type> m_substitution;
	std::vector<Constraint> m_constraints;
	UnionFind m_type_var_uf;

	int m_var_counter {0};
	int m_term_counter {0};
	int m_type_counter {0};
};
