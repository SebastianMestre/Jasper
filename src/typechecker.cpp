#include "typechecker.hpp"

#include "typechecker_types.hpp"
#include "ast.hpp"
#include "ast_allocator.hpp"

namespace TypeChecker {

TypeChecker::TypeChecker(AST::Allocator& allocator) : m_ast_allocator(&allocator) {
	// INVARIANT: we care only for the headers,
	// wether something's a var or a term and which one
	m_core.new_builtin_type_function(-1); // 0  | function
	m_core.new_builtin_type_function(0);  // 1  | int
	m_core.new_builtin_type_function(0);  // 2  | float
	m_core.new_builtin_type_function(0);  // 3  | string
	m_core.new_builtin_type_function(1);  // 4  | array
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
		auto var_id = new_hidden_var();

		{
			auto poly = m_core.new_poly(var_id, {var_id});
			declare_builtin_value("print", poly);
		}

		{
			auto array_mono_id =
			    m_core.new_term(BuiltinType::Array, {var_id}, "array");

			{
				auto term_mono_id = m_core.new_term(
				    BuiltinType::Function,
				    {array_mono_id, var_id, mono_unit()},
				    "[builtin] (array(<a>), a) -> unit");

				auto poly_id = m_core.new_poly(term_mono_id, {var_id});
				declare_builtin_value("array_append", poly_id);
			}
			{
				auto term_mono_id = m_core.new_term(
				    BuiltinType::Function,
				    {array_mono_id, mono_int()},
				    "[builtin] (array(<a>)) -> int");

				auto poly_id = m_core.new_poly(term_mono_id, {var_id});
				declare_builtin_value("size", poly_id);
			}
			{
				auto term_mono_id = m_core.new_term(
				    BuiltinType::Function,
				    {array_mono_id, array_mono_id, array_mono_id},
				    "[builtin] (array(<a>), array(<a>)) -> array(<a>)");

				auto poly_id = m_core.new_poly(term_mono_id, {var_id});
				declare_builtin_value("array_extend", poly_id);
			}
			{
				auto array_mono_id = m_core.new_term(
				    BuiltinType::Array, {mono_int()}, "array(<int>)");

				auto term_mono_id = m_core.new_term(
				    BuiltinType::Function,
				    {array_mono_id, mono_string(), mono_string()},
				    "[builtin] (array(<int>), string)) -> string");

				auto poly_id = m_core.new_poly(term_mono_id, {});
				declare_builtin_value("array_join", poly_id);
			}
			{
				auto term_mono_id = m_core.new_term(
				    BuiltinType::Function,
				    {array_mono_id, mono_int(), var_id},
				    "[builtin] (array(<a>), int) -> a");

				auto poly_id = m_core.new_poly(term_mono_id, {var_id});
				declare_builtin_value("array_at", poly_id);
			}
		}

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {var_id, var_id, var_id},
			    "[builtin] (a, a) -> a");

			auto poly_id = m_core.new_poly(term_mono_id, {var_id});

			declare_builtin_value("+", poly_id);
			declare_builtin_value("-", poly_id);
			declare_builtin_value("*", poly_id);
			declare_builtin_value("/", poly_id);
			declare_builtin_value(".", poly_id);
			declare_builtin_value("=", poly_id);
		}

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {var_id, var_id, mono_boolean()},
			    "[builtin] (a, a) -> Bool");

			auto poly_id = m_core.new_poly(term_mono_id, {var_id});

			declare_builtin_value( "<", poly_id);
			declare_builtin_value(">=", poly_id);
			declare_builtin_value( ">", poly_id);
			declare_builtin_value("<=", poly_id);
			declare_builtin_value("==", poly_id);
			declare_builtin_value("!=", poly_id);
		}

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function,
			    {mono_boolean(), mono_boolean(), mono_boolean()},
			    "[builtin] (Bool, Bool) -> Bool");

			auto poly_id = m_core.new_poly(term_mono_id, {});

			declare_builtin_value("&&", poly_id);
			declare_builtin_value("||", poly_id);
		}

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function, {mono_int()}, "[builtin] () -> Integer");
			auto poly_id = m_core.new_poly(term_mono_id, {});
			declare_builtin_value("read_integer", poly_id);
		}

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function, {mono_float()}, "[builtin] () -> Number");
			auto poly_id = m_core.new_poly(term_mono_id, {});
			declare_builtin_value("read_number", poly_id);
		}

		{
			auto term_mono_id = m_core.new_term(
			    BuiltinType::Function, {mono_string()}, "[builtin] () -> String");
			auto poly_id = m_core.new_poly(term_mono_id, {});
			declare_builtin_value("read_string", poly_id);
			declare_builtin_value("read_line", poly_id);
		}
	}

	declare_builtin_typefunc("int", BuiltinType::Int);
	declare_builtin_typefunc("float", BuiltinType::Float);
	declare_builtin_typefunc("string", BuiltinType::String);
	declare_builtin_typefunc("boolean", BuiltinType::Boolean);
	declare_builtin_typefunc("array", BuiltinType::Array);
}

MonoId TypeChecker::new_hidden_var() {
	return m_core.m_mono_core.new_var();
}

MonoId TypeChecker::new_var() {
	MonoId result = m_core.m_mono_core.new_var();
	m_env.current_scope().m_type_vars.insert(result);
	return result;
}

MetaTypeId TypeChecker::new_meta_var() {
	return m_core.m_meta_core.make_var_node();
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

void TypeChecker::bind_free_vars(MonoId mono) {
	std::unordered_set<MonoId> free_vars;
	m_core.gather_free_vars(mono, free_vars);
	for (MonoId var : free_vars) {
		if (!m_env.has_type_var(var)) {
			m_env.current_scope().m_type_vars.insert(var);
		}
	}
}

// Hindley-Milner [App], modified for multiple argument functions.
MonoId TypeChecker::rule_app(std::vector<MonoId> args_types, MonoId func_type) {
	MonoId return_type = m_core.m_mono_core.new_var();
	args_types.push_back(return_type);

	MonoId deduced_func_type =
	    m_core.new_term(BuiltinType::Function, std::move(args_types));

	m_core.m_mono_core.unify(func_type, deduced_func_type);

	return return_type;
}

AST::Declaration* TypeChecker::new_builtin_declaration(InternedString const& name) {
	m_builtin_declarations.push_back({});
	auto result = &m_builtin_declarations.back();
	result->m_identifier = name;
	return result;
}

void TypeChecker::declare_builtin_typefunc(
    InternedString const& name, TypeFunctionId typefunc) {
	auto decl = new_builtin_declaration(name);

	auto handle = m_ast_allocator->make<AST::TypeFunctionHandle>();
	handle->m_value = typefunc;
	decl->m_value = handle;
	decl->m_meta_type = m_core.m_meta_core.make_const_node(Tag::Func);
}

void TypeChecker::declare_builtin_value(InternedString const& name, PolyId poly_type) {
	auto decl = new_builtin_declaration(name);

	decl->m_decl_type = poly_type;
	decl->m_is_polymorphic = true;
	decl->m_meta_type = m_core.m_meta_core.make_const_node(Tag::Term);
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
