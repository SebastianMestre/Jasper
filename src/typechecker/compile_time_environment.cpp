#include "compile_time_environment.hpp"

#include "../algorithms/tarjan_solver.hpp"
#include "../ast.hpp"
#include "../log/log.hpp"
#include "typesystem.hpp"

#include <cassert>

namespace Frontend {

CompileTimeEnvironment::CompileTimeEnvironment() {}

CompileTimeEnvironment::Scope& CompileTimeEnvironment::current_scope() {
	return m_scopes.empty() ? m_global_scope : m_scopes.back();
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

bool CompileTimeEnvironment::has_type_var(MonoId var, TypeSystemCore& core) {
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
	auto found = scan_scope(m_global_scope, var);
	if (found)
		return true;

	for (auto& scope : m_scopes) {
		for (auto stored_var : scope.m_type_vars) {
			std::unordered_set<MonoId> free_vars;
			core.gather_free_vars(stored_var, free_vars);
			if (free_vars.count(var))
				return true;
		}
	}

	return false;
}

void CompileTimeEnvironment::bind_var_if_not_present(MonoId var, TypeSystemCore& core) {
	if (!has_type_var(var, core))
		bind_to_current_scope(var);
}

void CompileTimeEnvironment::bind_to_current_scope(MonoId var) {
	current_scope().m_type_vars.insert(var);
}

} // namespace Frontend
