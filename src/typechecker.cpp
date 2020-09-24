#include "typechecker.hpp"

#include "typed_ast.hpp"

#include <cassert>

namespace TypeChecker {

TypeChecker::TypeChecker() {
	m_core.new_builtin_type_function(-1); // 0  | function
	m_core.new_builtin_type_function(0);  // 1  | int
	m_core.new_builtin_type_function(0);  // 2  | float
	m_core.new_builtin_type_function(0);  // 3  | string
	m_core.new_builtin_type_function(1);  // 4  | array
	m_core.new_builtin_type_function(1);  // 5  | dictionary
	m_core.new_builtin_type_function(0);  // 6  | boolean
	m_core.new_builtin_type_function(0);  // 7  | unit

	m_core.new_term(1, {}, "builtin int");    // 0 | int(<>)
	m_core.new_term(2, {}, "builtin float");  // 1 | float(<>)
	m_core.new_term(3, {}, "builtin string"); // 2 | string(<>)
	m_core.new_term(6, {}, "builtin bool");   // 3 | boolean(<>)
	m_core.new_term(7, {}, "builtin unit");   // 4 | unit(<>)

	// TODO: put this in a better place
	// TODO: refactor, figure out a nice way to build types
	// HACK: this is an ugly hack. bear with me...

	{
		auto var = new_var();
		auto poly = m_core.new_poly(var, {var});
		m_env.declare_builtin("print", poly);
	}

	{
		auto var_id = new_hidden_var();

		auto array_mono_id = m_core.new_term(
		    BuiltinType::Array, {var_id}, "array");

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {array_mono_id, var_id, mono_unit()},
			    "[builtin] (array(<a>), a) -> unit");

			auto poly_id = m_core.new_poly(term_mono_id, {var_id});
			m_env.declare_builtin("array_append", poly_id);
		}
		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {array_mono_id, mono_int()},
			    "[builtin] (array(<a>)) -> int");

			auto poly_id = m_core.new_poly(term_mono_id, {var_id});
			m_env.declare_builtin("size", poly_id);
		}
		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {array_mono_id, array_mono_id, array_mono_id},
			    "[builtin] (array(<a>), array(<a>)) -> array(<a>)");

			auto poly_id = m_core.new_poly(term_mono_id, {var_id});
			m_env.declare_builtin("array_extend", poly_id);
		}
		{
			auto array_mono_id = m_core.new_term(
			    BuiltinType::Array,
			    {mono_int()},
			    "array(<int>)");

			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {array_mono_id,
			     mono_string(),
			     mono_string()},
			    "[builtin] (array(<int>), string)) -> string");

			auto poly_id = m_core.new_poly(term_mono_id, {});
			m_env.declare_builtin("array_join", poly_id);
		}
		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {array_mono_id, mono_int(), var_id},
			    "[builtin] (array(<a>), int) -> a");

			auto poly_id = m_core.new_poly(term_mono_id, {var_id});
			m_env.declare_builtin("array_at", poly_id);
		}
	}

	{
		auto var_id = new_hidden_var();

		// TODO: i use the same mono thrice... does this make sense?
		auto term_mono_id = m_core.new_term(
		    BuiltinType::Function,
		    {var_id, var_id, var_id},
		    "[builtin] (a, a) -> a");

		auto poly_id = m_core.new_poly(term_mono_id, {var_id});

		// TODO: re using the same PolyId... is this ok?
		// I think this is fine because we always do inst_fresh when we use a poly
		// , so it can't somehow get mutated
		m_env.declare_builtin("+", poly_id);
		m_env.declare_builtin("-", poly_id);
		m_env.declare_builtin("*", poly_id);
		m_env.declare_builtin("/", poly_id);
		m_env.declare_builtin(".", poly_id);
		m_env.declare_builtin("=", poly_id);
	}

	{
		auto var_id = new_hidden_var();

		auto term_mono_id = m_core.new_term(
		    BuiltinType::Function,
		    {var_id, var_id, mono_boolean()},
		    "[builtin] (a, a) -> Bool");

		auto poly_id = m_core.new_poly(term_mono_id, {var_id});

		m_env.declare_builtin("<", poly_id);
		m_env.declare_builtin("==", poly_id);
	}
}

MonoId TypeChecker::new_hidden_var() {
	return m_core.new_var();
}

MonoId TypeChecker::new_var() {
	MonoId result = m_core.new_var();
	m_env.current_scope().m_type_vars.insert(result);
	return result;
}

// qualifies all free variables in the given monotype
PolyId TypeChecker::generalize(MonoId mono) {
	std::unordered_set<MonoId> free_vars;
	m_core.gather_free_vars(mono, free_vars);

	std::vector<MonoId> new_vars;
	std::unordered_map<MonoId, MonoId> mapping;
	for (MonoId var : free_vars) {
		if (!m_env.has_type_var(var)) {
			auto fresh_var = new_hidden_var();
			new_vars.push_back(fresh_var);
			mapping[var] = fresh_var;
		}
	}

	MonoId base = m_core.inst_impl(mono, mapping);

	return m_core.new_poly(base, std::move(new_vars));
}

// Hindley-Milner [App], modified for multiple argument functions.
MonoId TypeChecker::rule_app(std::vector<MonoId> args_types, MonoId func_type) {
	MonoId return_type = m_core.new_var();
	args_types.push_back(return_type);

	MonoId deduced_func_type =
	    m_core.new_term(BuiltinType::Function, std::move(args_types));

	m_core.unify(func_type, deduced_func_type);

	return return_type;
}

MonoId TypeChecker::mono_int() {
	return 0;
}

MonoId TypeChecker::mono_float() {
	return 1;
}

MonoId TypeChecker::mono_string() {
	return 2;
}

MonoId TypeChecker::mono_boolean() {
	return 3;
}

MonoId TypeChecker::mono_unit() {
	return 4;
}

} // namespace TypeChecker
