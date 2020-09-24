#pragma once

#include "compile_time_environment.hpp"
#include "typesystem.hpp"

namespace TypeChecker {

struct TypeChecker {

	TypeSystemCore m_core;
	Frontend::CompileTimeEnvironment m_env;

	TypeChecker();

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

	MonoId new_hidden_var() {
		return m_core.new_var();
	}

	MonoId new_var() {
		MonoId result = m_core.new_var();
		m_env.current_scope().m_type_vars.insert(result);
		return result;
	}

	MonoId rule_var(PolyId poly);
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);
	MonoId rule_abs();
	MonoId rule_let(MonoId mono);
	MonoId rule_rec();
};

} // namespace TypeChecker
