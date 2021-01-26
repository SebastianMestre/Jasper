#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "typechecker_types.hpp"
#include "utils/interned_string.hpp"

namespace AST {

struct Declaration;
struct DeclarationList;
struct FunctionLiteral;
struct SequenceExpression;

} // namespace AST

namespace Frontend {

struct Scope {
	bool m_nested {false};
	std::unordered_map<InternedString, AST::Declaration*> m_vars;
	std::unordered_set<MonoId> m_type_vars;
};

struct CompileTimeEnvironment {
	Scope m_global_scope;
	std::vector<Scope> m_scopes;
	std::vector<AST::FunctionLiteral*> m_function_stack;
	std::vector<AST::SequenceExpression*> m_seq_expr_stack;
	AST::Declaration* m_current_decl {nullptr};

	// TODO: is this a good place to put this?
	std::vector<std::vector<AST::Declaration*>> declaration_components;

	CompileTimeEnvironment();

	void declare(AST::Declaration*);

	AST::Declaration* access(InternedString const&);

	AST::SequenceExpression* current_seq_expr();
	void enter_seq_expr(AST::SequenceExpression*);
	void exit_seq_expr();

	AST::FunctionLiteral* current_function();
	void enter_function(AST::FunctionLiteral*);
	void exit_function();

	AST::Declaration* current_top_level_declaration();
	void enter_top_level_decl(AST::Declaration*);
	void exit_top_level_decl();

	Scope& current_scope();
	void new_scope();
	void new_nested_scope();
	void end_scope();

	bool has_type_var(MonoId);

	void compute_declaration_order(AST::DeclarationList* ast);
};

} // namespace Frontend

