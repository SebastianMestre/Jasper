#include "typechecker.hpp"

#include "typed_ast.hpp"

#include <cassert>

namespace TypeChecker {

TypeChecker::TypeChecker() {
	m_core.type_function_data.push_back({ -1 }); // 0 | function
	m_core.type_function_data.push_back({  0 }); // 1 | int
	m_core.type_function_data.push_back({  0 }); // 2 | float
	m_core.type_function_data.push_back({  0 }); // 3 | string

	m_core.term_data.push_back({ 1, {}}); // 0 | int(<>)
	m_core.term_data.push_back({ 2, {}}); // 1 | float(<>)
	m_core.term_data.push_back({ 3, {}}); // 2 | string(<>)

	m_core.mono_data.push_back({ mono_type::Term, 0 }); // 0 | int(<>)
	m_core.mono_data.push_back({ mono_type::Term, 1 }); // 1 | float(<>)
	m_core.mono_data.push_back({ mono_type::Term, 2 }); // 2 | string(<>)
}

TypeFunctionId tf_function() { return 0; }

MonoId TypeChecker::mono_int() { return 0; }
MonoId TypeChecker::mono_float() { return 1; }
MonoId TypeChecker::mono_string() { return 2; }

MonoId TypeChecker::rule_var(PolyId poly) {
	return m_core.inst_fresh(poly);
}

// Hindley-Milner [App], modified for multiple argument functions.
MonoId TypeChecker::rule_app(std::vector<MonoId> args_types, MonoId func_type) {
	MonoId return_type = m_core.new_var();

	args_types.push_back(return_type);
	MonoId deduced_func_type
	    = m_core.new_term(tf_function(), std::move(args_types));

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
