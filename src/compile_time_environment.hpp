#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "typechecker_types.hpp"
#include "utils/interned_string.hpp"

namespace TypedAST {

struct Declaration;
struct FunctionLiteral;

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
	TypedAST::Declaration* m_current_decl {nullptr};

	CompileTimeEnvironment();

	void declare(InternedString const&, TypedAST::Declaration*);

	TypedAST::Declaration* access(InternedString const&);

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
};

} // namespace Frontend
