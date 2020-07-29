#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace TypedAST {

struct Declaration;
struct FunctionLiteral;

}

namespace Frontend {

struct Scope {
	bool m_nested { false };
	std::unordered_map<std::string, TypedAST::Declaration*> m_vars;
};

struct CompileTimeEnvironment {
	Scope m_global_scope;
	std::vector<Scope> m_scopes;
	std::vector<TypedAST::FunctionLiteral*> m_function_stack;

	CompileTimeEnvironment();

	void declare(std::string const&, TypedAST::Declaration*);
	TypedAST::Declaration* access(std::string const&);

	TypedAST::FunctionLiteral* current_function();
	void enter_function(TypedAST::FunctionLiteral*);
	void exit_function();

	Scope& current_scope();
	void new_scope();
	void new_nested_scope();
	void end_scope();
};

} // namespace Frontend
