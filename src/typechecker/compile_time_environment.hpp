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

struct TypeSystemCore;

namespace Frontend {

struct CompileTimeEnvironment {
	struct Scope {
		bool m_nested {false};
		std::unordered_set<MonoId> m_type_vars;
	};

	std::vector<Scope> m_scopes;
	std::vector<AST::FunctionLiteral*> m_function_stack;
	std::vector<AST::SequenceExpression*> m_seq_expr_stack;
	AST::Declaration* m_current_decl {nullptr};

	CompileTimeEnvironment();

	Scope& current_scope();
	void new_scope();
	void new_nested_scope();
	void end_scope();

	bool has_type_var(MonoId, TypeSystemCore&);
	void bind_var_if_not_present(MonoId, TypeSystemCore&);
	void bind_to_current_scope(MonoId);

	void compute_declaration_order(AST::Program* ast);

	Scope& global_scope();

private:
	Scope m_global_scope;
};

} // namespace Frontend

