#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace TypedAST {

struct Declaration;

}

namespace Frontend {

struct Scope {
	bool m_nested { false };
	std::unordered_map<std::string, TypedAST::Declaration*> m_vars;
};

struct CompileTimeEnvironment {
	Scope m_global_scope;
	std::vector<Scope> m_scopes;

	CompileTimeEnvironment();

	Scope& current_scope();

	void declare(std::string const&, TypedAST::Declaration*);
	TypedAST::Declaration* access(std::string const&);

	void new_scope();
	void new_nested_scope();
	void end_scope();
};

} // namespace Frontend
