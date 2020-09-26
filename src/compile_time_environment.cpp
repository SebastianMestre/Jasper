#include "compile_time_environment.hpp"

#include "typed_ast.hpp"

#include <cassert>

namespace Frontend {

CompileTimeEnvironment::CompileTimeEnvironment() {}

Scope& CompileTimeEnvironment::current_scope() {
	return m_scopes.empty() ? m_global_scope : m_scopes.back();
}

void CompileTimeEnvironment::declare(
    std::string const& name, TypedAST::Declaration* decl) {
	// current_scope().m_vars[name] = decl;
	current_scope().m_vars.insert({name, decl});
}

TypedAST::Declaration* CompileTimeEnvironment::access(std::string const& name) {
	auto scan_scope = [](Scope& scope, std::string const& name) -> TypedAST::Declaration* {
		auto it = scope.m_vars.find(name);
		if (it != scope.m_vars.end())
			return it->second;
		return nullptr;
	};

	// scan nested scopes from the inside out
	for (int i = m_scopes.size(); i--;) {
		auto ptr = scan_scope(m_scopes[i], name);
		if (ptr)
			return ptr;
		if (!m_scopes[i].m_nested)
			break;
	}

	// fall back to global scope lookup
	return scan_scope(m_global_scope, name);
}

void CompileTimeEnvironment::new_scope() {
	m_scopes.push_back({false});
}

void CompileTimeEnvironment::new_nested_scope() {
	m_scopes.push_back({true});
}

void CompileTimeEnvironment::end_scope() {
	m_scopes.pop_back();
}

TypedAST::FunctionLiteral* CompileTimeEnvironment::current_function() {
	return m_function_stack.empty() ? nullptr : m_function_stack.back();
}

void CompileTimeEnvironment::enter_function(TypedAST::FunctionLiteral* func) {
	m_function_stack.push_back(func);
}

void CompileTimeEnvironment::exit_function() {
	m_function_stack.pop_back();
}

bool CompileTimeEnvironment::has_type_var(MonoId var) {
	// TODO: check that the given mono is actually a var

	auto scan_scope = [](Scope& scope, MonoId var) -> bool {
		return scope.m_type_vars.count(var) != 0;
	};

	// scan nested scopes from the inside out
	for (int i = m_scopes.size(); i--;) {
		auto found = scan_scope(m_scopes[i], var);
		if (found)
			return true;
		if (!m_scopes[i].m_nested)
			break;
	}

	// fall back to global scope lookup
	return scan_scope(m_global_scope, var);
}

TypedAST::Declaration* CompileTimeEnvironment::current_top_level_declaration() {
	return m_current_decl;
}

void CompileTimeEnvironment::enter_top_level_decl(TypedAST::Declaration* decl) {
	assert(!m_current_decl);
	m_current_decl = decl;
}

void CompileTimeEnvironment::exit_top_level_decl() {
	assert(m_current_decl);
	m_current_decl = nullptr;
}

} // namespace Frontend
