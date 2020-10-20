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

	std::vector<std::string> fields;
	std::unordered_map<std::string, MonoId> structure;

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

	Unification::Core m_tf_core;
	std::vector<TypeFunctionData> m_type_functions;

	std::vector<PolyData> poly_data;

	Unification::Core m_meta_core;

	TypeSystemCore();

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> args,
	    char const* tag = nullptr);

	PolyId new_poly(MonoId mono, std::vector<MonoId> vars);

	TypeFunctionId new_builtin_type_function(int arguments);
	TypeFunctionId new_dummy_type_function
	    ( TypeFunctionTag type
	    , std::vector<std::string> fields
	    , std::unordered_map<std::string, MonoId> structure);
	
	// qualifies all unbound variables in the given monotype
	void gather_free_vars(MonoId mono, std::unordered_set<MonoId>& free_vars);

	MonoId inst_impl(MonoId mono, std::unordered_map<MonoId, MonoId> const& mapping);
	MonoId inst_with(PolyId poly, std::vector<MonoId> const& vals);
	MonoId inst_fresh(PolyId poly);
};
