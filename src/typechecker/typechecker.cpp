#include "typechecker.hpp"

#include "../algorithms/tarjan_solver.hpp"
#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "typechecker_types.hpp"

namespace TypeChecker {

TypeChecker::TypeChecker(AST::Allocator& allocator)
    : m_ast_allocator(&allocator) {
	// INVARIANT: we care only for the headers,
	// wether something's a var or a term and which one
	core().new_builtin_type_function(-1); // 0  | function
	core().new_builtin_type_function(0);  // 1  | int
	core().new_builtin_type_function(0);  // 2  | float
	core().new_builtin_type_function(0);  // 3  | string
	core().new_builtin_type_function(1);  // 4  | array
	core().new_builtin_type_function(0);  // 5  | boolean
	core().new_builtin_type_function(0);  // 6  | unit

	core().new_term(BuiltinType::Int, {});      // 0 | int<::>
	core().new_term(BuiltinType::Float, {});    // 1 | float<::>
	core().new_term(BuiltinType::String, {});   // 2 | string<::>
	core().new_term(BuiltinType::Boolean, {});  // 3 | boolean<::>
	core().new_term(BuiltinType::Unit, {});     // 4 | unit<::>

	// TODO: put this in a better place
	// TODO: refactor, figure out a nice way to build types
	// HACK: this is an ugly hack. bear with me...

	{
		auto var_ty = new_var();
		VarId var_id = core().get_var_id(var_ty);

		{
			auto poly = core().forall({var_id}, var_ty);
			declare_builtin_value("print", poly);
		}

		{
			auto array_ty = core().array(var_ty);

			{
				auto ty = core().fun({array_ty, var_ty}, mono_unit());
				auto poly = core().forall({var_id}, ty);
				declare_builtin_value("array_append", poly);
			}

			{
				auto ty = core().fun({array_ty}, mono_int());
				auto poly = core().forall({var_id}, ty);
				declare_builtin_value("size", poly);
			}

			{
				auto ty = core().fun({array_ty, array_ty}, array_ty);
				auto poly = core().forall({var_id}, ty);
				declare_builtin_value("array_extend", poly);
			}

			{
				auto array_ty = core().array(mono_int());
				auto ty = core().fun({array_ty, mono_string()}, mono_string());
				auto poly = core().forall({}, ty);

				declare_builtin_value("array_join", poly);
			}

			{
				auto ty = core().fun({array_ty, mono_int()}, var_ty);
				auto poly = core().forall({var_id}, ty);

				declare_builtin_value("array_at", poly);
			}
		}

		{
			auto ty = core().fun({var_ty, var_ty}, var_ty);
			auto poly = core().forall({var_id}, ty);

			declare_builtin_value("+", poly);
			declare_builtin_value("-", poly);
			declare_builtin_value("*", poly);
			declare_builtin_value("/", poly);
			declare_builtin_value(".", poly);
			declare_builtin_value("=", poly);
		}

		{
			auto ty = core().fun({var_ty, var_ty}, mono_boolean());
			auto poly = core().forall({var_id}, ty);

			declare_builtin_value( "<", poly);
			declare_builtin_value(">=", poly);
			declare_builtin_value( ">", poly);
			declare_builtin_value("<=", poly);
			declare_builtin_value("==", poly);
			declare_builtin_value("!=", poly);
		}

		{
			auto ty = core().fun({mono_boolean(), mono_boolean()}, mono_boolean());
			auto poly = core().forall({}, ty);

			declare_builtin_value("&&", poly);
			declare_builtin_value("||", poly);
		}

		{
			auto ty = core().fun({}, mono_int());
			auto poly = core().forall({}, ty);

			declare_builtin_value("read_integer", poly);
		}

		{
			auto ty = core().fun({}, mono_float());
			auto poly = core().forall({}, ty);

			declare_builtin_value("read_number", poly);
		}

		{
			auto ty = core().fun({}, mono_string());
			auto poly = core().forall({}, ty);

			declare_builtin_value("read_string", poly);
			declare_builtin_value("read_line", poly);
		}

	}

	declare_builtin_typefunc("int", BuiltinType::Int);
	declare_builtin_typefunc("float", BuiltinType::Float);
	declare_builtin_typefunc("string", BuiltinType::String);
	declare_builtin_typefunc("boolean", BuiltinType::Boolean);
	declare_builtin_typefunc("array", BuiltinType::Array);
}

MonoId TypeChecker::new_var() {
	return core().ll_new_var();
}

AST::Declaration* TypeChecker::new_builtin_declaration(InternedString const& name) {
	m_builtin_declarations.push_back({});
	auto result = &m_builtin_declarations.back();
	result->m_identifier = name;
	return result;
}

void TypeChecker::declare_builtin_typefunc(InternedString const& name, TypeFunc typefunc) {
	auto decl = new_builtin_declaration(name);

	auto handle = m_ast_allocator->make<AST::BuiltinTypeFunction>();
	handle->m_value = typefunc;
	decl->m_value = handle;
	handle->m_meta_type = decl->m_meta_type = MetaType::TypeFunction;
}

void TypeChecker::declare_builtin_value(InternedString const& name, PolyId poly_type) {
	auto decl = new_builtin_declaration(name);

	decl->m_decl_type = poly_type;
	decl->m_is_polymorphic = true;
	decl->m_meta_type = MetaType::Term;
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

std::vector<std::vector<AST::Declaration*>> const& TypeChecker::declaration_order() const {
	return m_declaration_components;
}

void TypeChecker::compute_declaration_order(AST::Program* ast) {

	std::unordered_map<AST::Declaration*, int> decl_to_index;
	std::vector<AST::Declaration*> index_to_decl;

	// assign a unique int to every top level declaration
	int i = 0;
	for (auto& decl : ast->m_declarations) {
		index_to_decl.push_back(&decl);
		decl_to_index.insert({&decl, i});
		i += 1;
	}

	// build up the explicit declaration graph
	TarjanSolver solver(index_to_decl.size());
	for (auto kv : decl_to_index) {
		auto decl = kv.first;
		auto u = kv.second;
		for (auto other : decl->m_references) {
			auto it = decl_to_index.find(other);
			if (it != decl_to_index.end()) {
				int v = it->second;
				solver.add_edge(u, v);
			}
		}
	}

	// compute strongly connected components
	solver.solve();

	auto const& comps = solver.vertices_of_components();
	std::vector<AST::Declaration*> decl_comp;
	for (auto const& comp : comps) {
		decl_comp.clear();
		decl_comp.reserve(comp.size());
		for (int u : comp)
			decl_comp.push_back(index_to_decl[u]);

		m_declaration_components.push_back(std::move(decl_comp));
	}
}

} // namespace TypeChecker
