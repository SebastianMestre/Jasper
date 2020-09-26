#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "chunked_array.hpp"
#include "typechecker_types.hpp"

namespace TypedAST {

struct Declaration;
struct FunctionLiteral;

} // namespace TypedAST

namespace Frontend {

struct Scope {
	bool m_nested {false};
	std::unordered_map<std::string, TypedAST::Declaration*> m_vars;
	std::unordered_set<MonoId> m_type_vars;
};

struct CompileTimeEnvironment {
	Scope m_global_scope;
	std::vector<Scope> m_scopes;
	std::vector<TypedAST::FunctionLiteral*> m_function_stack;
	TypedAST::Declaration* m_current_decl {nullptr};

	ChunkedArray<TypedAST::Declaration> m_builtin_declarations;

	CompileTimeEnvironment();

	void declare(std::string const&, TypedAST::Declaration*);
	void declare_builtin(std::string const&, MetaTypeId, PolyId);

	TypedAST::Declaration* access(std::string const&);

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
