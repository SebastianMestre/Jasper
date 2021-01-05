#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "typechecker_types.hpp"
#include "utils/interned_string.hpp"

namespace TypedAST {

struct Declaration;
struct DeclarationList;
struct FunctionLiteral;
struct SequenceExpression;

} // namespace TypedAST

namespace Frontend {

struct Scope {
	bool m_nested {false};
	std::unordered_map<InternedString, TypedAST::Declaration*> m_vars;
	std::unordered_set<MonoId> m_type_vars;
};

struct CompileTimeEnvironment {
	Scope m_global_scope;
	std::vector<Scope> m_scopes;
	std::vector<TypedAST::FunctionLiteral*> m_function_stack;
	std::vector<TypedAST::SequenceExpression*> m_seq_expr_stack;
	TypedAST::Declaration* m_current_decl {nullptr};

	// TODO: is this a good place to put this?
	std::vector<std::vector<TypedAST::Declaration*>> declaration_components;

	CompileTimeEnvironment();

	void declare(TypedAST::Declaration*);

	TypedAST::Declaration* access(InternedString const&);

	TypedAST::SequenceExpression* current_seq_expr();
	void enter_seq_expr(TypedAST::SequenceExpression*);
	void exit_seq_expr();

	TypedAST::FunctionLiteral* current_function();
	void enter_function(TypedAST::FunctionLiteral*);
	void exit_function();

	TypedAST::Declaration* current_top_level_declaration();
	void enter_top_level_decl(TypedAST::Declaration*);
	void exit_top_level_decl();

	Scope& current_scope();
	void new_scope();
	void new_nested_scope();
	void end_scope();

	bool has_type_var(MonoId);

	void compute_declaration_order(TypedAST::DeclarationList* ast);
};

} // namespace Frontend

