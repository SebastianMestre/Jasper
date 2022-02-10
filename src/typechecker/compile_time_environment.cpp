#include "compile_time_environment.hpp"

#include "../algorithms/tarjan_solver.hpp"
#include "../ast.hpp"
#include "../log/log.hpp"

#include <cassert>

namespace Frontend {

CompileTimeEnvironment::CompileTimeEnvironment() {
	m_scopes.push_back({});
}

CompileTimeEnvironment::Scope& CompileTimeEnvironment::current_scope() {
	return m_scopes.empty() ? global_scope() : m_scopes.back();
}

void CompileTimeEnvironment::new_scope() {
	m_scopes.push_back({});
}

void CompileTimeEnvironment::new_nested_scope() {
	m_scopes.push_back({});
}

void CompileTimeEnvironment::end_scope() {
	assert(m_scopes.size() > 1);
	m_scopes.pop_back();
}

void CompileTimeEnvironment::bind_to_current_scope(MonoId var) {
	current_scope().m_type_vars.insert(var);
}

CompileTimeEnvironment::Scope& CompileTimeEnvironment::global_scope() {
	return m_scopes[0];
}

std::vector<CompileTimeEnvironment::Scope> const& CompileTimeEnvironment::scopes() {
	return m_scopes;
}

} // namespace Frontend
