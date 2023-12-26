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

	core().new_term(BuiltinType::Int, {}, "builtin int");       // 0 | int<::>
	core().new_term(BuiltinType::Float, {}, "builtin float");   // 1 | float<::>
	core().new_term(BuiltinType::String, {}, "builtin string"); // 2 | string<::>
	core().new_term(BuiltinType::Boolean, {}, "builtin bool");  // 3 | boolean<::>
	core().new_term(BuiltinType::Unit, {}, "builtin unit");     // 4 | unit<::>

	// TODO: put this in a better place
	// TODO: refactor, figure out a nice way to build types
	// HACK: this is an ugly hack. bear with me...

	auto forall = [&](std::vector<VarId> quantified_vars, MonoId inner_ty) -> PolyId {
		return core().new_poly(inner_ty, std::move(quantified_vars));
	};

	auto fun_ty = [&](std::vector<MonoId> args_tys, MonoId return_ty, char const* debug_info = nullptr) -> MonoId {
		args_tys.push_back(return_ty);
		return core().new_term(BuiltinType::Function, std::move(args_tys), debug_info);
	};

	auto array_ty = [&](MonoId elem_ty, char const* debug_info = nullptr) -> MonoId {
		return core().new_term(BuiltinType::Array, {elem_ty}, "array");
	};

	{
		MonoId a_ty = new_var();
		VarId a_var = core().get_var_id(a_ty);

		MonoId array_a_ty = array_ty(a_ty, "array");

		// print;s type is hard to express, so we just give it the bottom type
		declare_builtin_value("print", forall({a_var}, a_ty));

		{

			{
				auto term_ty = fun_ty({array_a_ty, a_ty}, mono_unit(), "[builtin] (array(<a>), a) -> unit");
				declare_builtin_value("array_append", forall({a_var}, term_ty));
			}

			{
				auto term_ty = fun_ty({array_a_ty}, mono_int(), "[builtin] (array(<a>)) -> int");
				declare_builtin_value("size", forall({a_var}, term_ty));
			}

			{
				auto term_ty = fun_ty({array_a_ty, array_a_ty}, array_a_ty, "[builtin] (array(<a>), array(<a>)) -> array(<a>)");
				declare_builtin_value("array_extend", forall({a_var}, term_ty));
			}

			{
				auto array_int_ty = array_ty(mono_int(), "array(<int>)");
				auto term_ty = fun_ty({array_int_ty, mono_string()}, mono_string(), "[builtin] (array(<int>), string)) -> string");
				declare_builtin_value("array_join", forall({}, term_ty));
			}

			{
				auto term_ty = fun_ty({array_a_ty, mono_int()}, a_ty, "[builtin] (array(<a>), int) -> a");
				declare_builtin_value("array_at", forall({a_var}, term_ty));
			}
		}

		{
			auto term_ty = fun_ty({a_ty, a_ty}, a_ty, "[builtin] (a, a) -> a");
			auto poly_id = forall({a_var}, term_ty);
			declare_builtin_value("+", poly_id);
			declare_builtin_value("-", poly_id);
			declare_builtin_value("*", poly_id);
			declare_builtin_value("/", poly_id);
			declare_builtin_value(".", poly_id);
			declare_builtin_value("=", poly_id);
		}

		{
			auto term_ty = fun_ty({a_ty, a_ty}, mono_boolean(), "[builtin] (a, a) -> Bool");
			auto poly_id = forall({a_var}, term_ty);
			declare_builtin_value( "<", poly_id);
			declare_builtin_value(">=", poly_id);
			declare_builtin_value( ">", poly_id);
			declare_builtin_value("<=", poly_id);
			declare_builtin_value("==", poly_id);
			declare_builtin_value("!=", poly_id);
		}

		{
			auto term_ty = fun_ty({mono_boolean(), mono_boolean()}, mono_boolean(), "[builtin] (Bool, Bool) -> Bool");
			auto poly_id = forall({}, term_ty);
			declare_builtin_value("&&", poly_id);
			declare_builtin_value("||", poly_id);
		}

		{
			auto term_ty = fun_ty({}, mono_int(), "[builtin] () -> Integer");
			declare_builtin_value("read_integer", forall({}, term_ty));
		}

		{
			auto term_ty = fun_ty({}, mono_float(), "[builtin] () -> Number");
			declare_builtin_value("read_number", forall({}, term_ty));
		}

		{
			auto term_ty = fun_ty({}, mono_string(), "[builtin] () -> String");
			auto poly_id = forall({}, term_ty);
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

MonoId TypeChecker::new_var() {
	return core().ll_new_var();
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
