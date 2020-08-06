#include "typesystem.hpp"

namespace TypeChecker {

struct TypeChecker {

	TypeSystemCore m_core;

	TypeChecker();

	MonoId rule_var(PolyId poly);
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);
	MonoId rule_abs();
	MonoId rule_let(MonoId mono);
	MonoId rule_rec();
};

}
