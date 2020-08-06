#include "typechecker.hpp"

#include "typed_ast.hpp"

#include <cassert>

namespace TypeChecker {

TypeChecker::TypeChecker() {
	// arrow, for functions, id 0
	m_core.type_function_data.push_back({ -1 });
}

TypeFunctionId arrow_type_function() {
	return 0;
}

MonoId TypeChecker::rule_var(PolyId poly) {
	return m_core.inst_fresh(poly);
}

// Hindley-Milner [App], modified for multiple argument functions.
MonoId TypeChecker::rule_app(std::vector<MonoId> args_types, MonoId func_type) {
	MonoId return_type = m_core.new_var();

	args_types.push_back(return_type);
	MonoId deduced_func_type
	    = m_core.new_term(arrow_type_function(), std::move(args_types));

	m_core.unify(func_type, deduced_func_type);

	return return_type;
}

// TODO
MonoId TypeChecker::rule_abs() {
	assert(0);
}

// TODO
MonoId TypeChecker::rule_let(MonoId mono) {
	assert(0);
	// return m_core.generalize(mono);
}

// TODO
MonoId TypeChecker::rule_rec() {
	assert(0);
}

}
