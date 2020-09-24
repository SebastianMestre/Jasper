#include "compile_time_environment.hpp"

#include "typed_ast.hpp"

#include <cassert>

namespace Frontend {

Binding::Binding(TypedAST::Declaration* decl)
    : m_type {BindingTag::Declaration}
    , m_decl {decl} {}

Binding::Binding(TypedAST::FunctionLiteral* func, int arg_index)
    : m_type {BindingTag::Argument}
    , m_func {func}
    , m_arg_index {arg_index} {}

TypedAST::Declaration* Binding::get_decl() {
	assert(m_type == BindingTag::Declaration);
	return m_decl;
}

TypedAST::FunctionArgument& Binding::get_arg() {
	assert(m_type == BindingTag::Argument);
	return m_func->m_args[m_arg_index];
}

TypedAST::FunctionLiteral* Binding::get_func() {
	assert(m_type == BindingTag::Argument);
	return m_func;
}

CompileTimeEnvironment::CompileTimeEnvironment() {}

Scope& CompileTimeEnvironment::current_scope() {
	return m_scopes.empty() ? m_global_scope : m_scopes.back();
}

void CompileTimeEnvironment::declare(
    std::string const& name, TypedAST::Declaration* decl) {
	// current_scope().m_vars[name] = decl;
	current_scope().m_vars.insert({name, decl});
}

void CompileTimeEnvironment::declare_arg(
    std::string const& name, TypedAST::FunctionLiteral* func, int arg_index) {
	current_scope().m_vars.insert({name, {func, arg_index}});
}

void CompileTimeEnvironment::declare_builtin(std::string const& name, PolyId poly) {
	m_builtin_declarations.push_back({});
	TypedAST::Declaration* decl = &m_builtin_declarations.back();
	decl->m_decl_type = poly;
	decl->m_is_polymorphic = true;
	declare(name, decl);
}

Binding* CompileTimeEnvironment::access_binding(std::string const& name) {
	auto scan_scope = [](Scope& scope, std::string const& name) -> Binding* {
		auto it = scope.m_vars.find(name);
		if (it != scope.m_vars.end())
			return &it->second;
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

TypedAST::Declaration* CompileTimeEnvironment::access(std::string const& name) {
	auto binding = access_binding(name);
	return !binding ? nullptr : binding->get_decl();
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
