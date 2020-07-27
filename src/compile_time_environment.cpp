#include "compile_time_environment.hpp"

#include "typed_ast.hpp"

namespace Frontend {

CompileTimeEnvironment::CompileTimeEnvironment() {
	// TODO: put this in a better place
	// HACK: this is an ugly hack. bear with me...
	TypedAST::Declaration dummy;
	declare("size", &dummy);
	declare("print", &dummy);
	declare("array_append", &dummy);
	declare("array_extend", &dummy);
	declare("array_join", &dummy);

	declare("+", &dummy);
	declare("-", &dummy);
	declare("*", &dummy);
	declare("/", &dummy);
	declare("<", &dummy);
	declare("=", &dummy);
	declare("==", &dummy);
	declare(".", &dummy);
};

Scope& CompileTimeEnvironment::current_scope() {
	if (m_scopes.empty())
		return m_global_scope;
	else
		return m_scopes.back();
}

void CompileTimeEnvironment::declare(std::string const& name, TypedAST::Declaration* decl) {
	current_scope().m_vars[name] = decl;
}

TypedAST::Declaration* CompileTimeEnvironment::access(std::string const& name) {
	auto scan_scope
	    = [](Scope& scope, std::string const& name) -> TypedAST::Declaration* {
		auto it = scope.m_vars.find(name);
		if (it != scope.m_vars.end())
			return it->second;
		return nullptr;
	};

	// scan nested scopes from the inside out
	for(int i = m_scopes.size(); i--;){
		auto ptr = scan_scope(m_scopes[i], name);
		if (ptr) return ptr;
		if (!m_scopes[i].m_nested) break;
	}

	// fall back to global scope lookup
	return scan_scope(m_global_scope, name);
}

void CompileTimeEnvironment::new_scope() {
	m_scopes.push_back({ false });
}

void CompileTimeEnvironment::new_nested_scope() {
	m_scopes.push_back({ true });
}

void CompileTimeEnvironment::end_scope() {
	m_scopes.pop_back();
}

} // namespace Frontend
