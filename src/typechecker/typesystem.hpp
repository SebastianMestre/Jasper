#pragma once

#include <unordered_map>
#include <set>
#include <map>
#include <vector>

#include "../algorithms/union_find.hpp"
#include "../log/log.hpp"
#include "../utils/interned_string.hpp"
#include "typechecker_types.hpp"

// Type function strength is an ad-hoc concept, specific to our implementation
// of unification.
// If a typefunc has 'None' strength, its data is not even considered for
// unification.
// If it has 'Full' strength, we only accept exact matches during unification.
// We don't allow unifying two different full-strength type functions
enum class TypeFunctionStrength { None, Full };

enum class VarId {};

inline bool operator<(VarId a, VarId b) {
	return static_cast<int>(a) < static_cast<int>(b);
}

inline bool operator==(VarId a, VarId b) {
	return static_cast<int>(a) == static_cast<int>(b);
}

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

	TypeFunctionStrength strength;
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

	int ll_new_var(const char* debug = nullptr);
	void ll_unify(int i, int j);

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> args,
	    char const* tag = nullptr);

	PolyId new_poly(MonoId mono, std::vector<VarId> vars);

	TypeFunctionId new_builtin_type_function(int arguments);
	TypeFunctionId new_type_function(
	    TypeFunctionTag type,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure);

	TypeFunctionId new_type_function_for_ct_eval1(
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure) {
		return new_type_function(
		    TypeFunctionTag::Record, std::move(fields), std::move(structure));
	}

	TypeFunctionId new_type_function_for_ct_eval2(
	    std::unordered_map<InternedString, MonoId> structure) {
		return new_type_function(TypeFunctionTag::Variant, {}, std::move(structure));
	}

	// qualifies all unbound variables in the given monotype
	void gather_free_vars(MonoId mono, std::set<VarId>& free_vars);

	MonoId inst_impl(MonoId mono, std::map<VarId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);

	TypeFunctionData& type_function_data_of(MonoId);
	TypeFunctionId new_type_function_var();
	void unify_type_function(TypeFunctionId, TypeFunctionId);

	void add_record_constraint(VarId v) {
		int i = static_cast<int>(v);
		auto& current_shape = m_constraints[i].shape;
		if (current_shape == Constraint::Shape::Unknown) {
			current_shape = Constraint::Shape::Record;
		} else if (current_shape == Constraint::Shape::Record) {
			// OK!
		} else {
			Log::fatal() << "Some typevar is both union and struct";
		}
	}

	void add_variant_constraint(VarId v) {
		int i = static_cast<int>(v);
		auto& current_shape = m_constraints[i].shape;
		if (current_shape == Constraint::Shape::Unknown) {
			current_shape = Constraint::Shape::Variant;
		} else if (current_shape == Constraint::Shape::Variant) {
			// OK!
		} else {
			Log::fatal() << "Some typevar is both variant and struct";
		}
	}

	void add_field_constraint(VarId v, InternedString x, MonoId t) {
		int i = static_cast<int>(v);
		if (m_constraints[i].structure.count(x)) {
			ll_unify(m_constraints[i].structure[x], t);
		} else {
			m_constraints[i].structure[x] = t;
		}
	}
private:

	enum class Tag { Var, Term, };

	struct NodeHeader {
		Tag tag;
		int data_idx;
		const char* debug {nullptr};
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

	int ll_new_term(int f, std::vector<int> args = {}, const char* debug = nullptr);
	void point_type_function_at_another(TypeFunctionId, TypeFunctionId);
	void unify_type_function_data(TypeFunctionData&, TypeFunctionData&);
	int compute_new_argument_count(TypeFunctionData const&, TypeFunctionData const&) const;
	TypeFunctionData& get_type_function_data(TypeFunctionId);
	TypeFunctionId find_type_function(TypeFunctionId);

	TypeFunctionId create_type_function(
	    TypeFunctionTag tag,
	    int arity,
	    std::vector<InternedString> fields,
	    std::unordered_map<InternedString, MonoId> structure,
	    TypeFunctionStrength);

	void establish_substitution(VarId var_id, int type_id);

	bool occurs(VarId v, MonoId i);
	bool equals_var(MonoId t, VarId v);
	void unify_vars_left_to_right(VarId vi, VarId vj);
	void combine_constraints_left_to_right(VarId vi, VarId vj);
	bool satisfies(MonoId t, Constraint const& c);

public:
	VarId get_var_id(MonoId i);

private:
	// per-func data
	std::vector<TypeFunctionData> m_type_functions;
	UnionFind m_type_function_uf;

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
