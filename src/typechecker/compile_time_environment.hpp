#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "typechecker_types.hpp"
#include "../utils/interned_string.hpp"

namespace AST {

struct Declaration;
struct Program;
struct FunctionLiteral;
struct SequenceExpression;

} // namespace AST

namespace Frontend {

struct CompileTimeEnvironment {
	struct Scope {
		std::unordered_set<MonoId> m_type_vars;
	};

	std::vector<Scope> m_scopes;

	CompileTimeEnvironment();

	Scope& current_scope();
	void new_scope();
	void new_nested_scope();
	void end_scope();

	void bind_to_current_scope(MonoId);

	void compute_declaration_order(AST::Program* ast);

	Scope& global_scope();
	std::vector<Scope> const& scopes();
};

} // namespace Frontend

