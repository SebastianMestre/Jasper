#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../algorithms/union_find.hpp"
#include "../utils/interned_string.hpp"
#include "meta_unifier.hpp"
#include "typechecker_types.hpp"

// Type function strength is an ad-hoc concept, specific to our implementation
// of unification.
// If a typefunc has 'None' strength, its data is not even considered for
// unification.
// If it has 'Half' strength, its data is considered to be incomplete, so we
// allow adding to it, but not removing.
// If it has 'Full' strength, we only accept exact matches during unification.
// We don't allow unifying two different full-strength type functions
enum class TypeFunctionStrength { None, Half, Full };

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
	std::vector<MonoId> vars;
};

struct TypeSystemCore {

	TypeSystemCore();

	int ll_new_var(const char* debug = nullptr);
	void ll_unify(int i, int j);

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
	TypeFunctionId new_type_function_var();
	void unify_type_function(TypeFunctionId, TypeFunctionId);

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

	void ll_unify_function(int, int);

	bool ll_occurs(int v, int i);

	int ll_find(int i);
	int ll_find_term(int i);
	int ll_find_function(int i);



	bool ll_is_var(int i);
	bool ll_is_term(int i);

	int ll_new_term(int f, std::vector<int> args = {}, const char* debug = nullptr);
	void ll_print_node(int node_header, int d = 0);
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

public:
	MetaUnifier m_meta_core;
private:
	std::vector<TypeFunctionData> m_type_functions;
	std::vector<PolyData> poly_data;
	UnionFind m_type_function_uf;
	std::vector<NodeHeader> ll_node_header;
	std::vector<TermData> ll_term_data;
};
