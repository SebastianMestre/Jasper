#include "typesystem.hpp"

namespace TypeChecker {

struct TypeChecker {

	TypeSystemCore m_core;

	TypeChecker();

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

	MonoId new_var() {
		return m_core.new_var();
	}

	MonoId rule_var(PolyId poly);
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);
	MonoId rule_abs();
	MonoId rule_let(MonoId mono);
	MonoId rule_rec();
};

}
