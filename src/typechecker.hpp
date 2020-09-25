#pragma once

#include "compile_time_environment.hpp"
#include "typesystem.hpp"

namespace TypeChecker {

struct TypeChecker {

	TypeSystemCore m_core;
	Frontend::CompileTimeEnvironment m_env;

	TypeChecker();

	PolyId generalize(MonoId mono);

	MonoId new_hidden_var();
	MonoId new_var();

	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

};

} // namespace TypeChecker
