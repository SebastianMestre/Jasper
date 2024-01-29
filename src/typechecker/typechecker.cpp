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

	auto a = new_var();
	auto aid = core().get_var_id(a);

	auto array_a = core().array(a);

	auto array_int = core().array(integer());

	auto forall_a = [&](Type t) {
		return this->core().forall({aid}, t);
	};

	auto mono = [&](Type t) {
		return this->core().forall({}, t);
	};

	auto fun = [&](std::vector<Type> args, Type res) {
		return this->core().fun(std::move(args), res);
	};

	declare_builtin_value("print", forall_a(a));

	declare_builtin_value("array_append", forall_a(fun({array_a, a}, unit())));
	declare_builtin_value("array_extend", forall_a(fun({array_a, array_a}, array_a)));
	declare_builtin_value("array_join",   mono(fun({array_int, string()}, string())));
	declare_builtin_value("array_at",     forall_a(fun({array_a, integer()}, a)));
	declare_builtin_value("size",         forall_a(fun({array_a}, integer())));

	declare_builtin_value("+", forall_a(fun({a, a}, a)));
	declare_builtin_value("-", forall_a(fun({a, a}, a)));
	declare_builtin_value("*", forall_a(fun({a, a}, a)));
	declare_builtin_value("/", forall_a(fun({a, a}, a)));
	declare_builtin_value(".", forall_a(fun({a, a}, a)));
	declare_builtin_value("=", forall_a(fun({a, a}, a)));

	declare_builtin_value("<",  forall_a(fun({a, a}, boolean())));
	declare_builtin_value(">=", forall_a(fun({a, a}, boolean())));
	declare_builtin_value(">",  forall_a(fun({a, a}, boolean())));
	declare_builtin_value("<=", forall_a(fun({a, a}, boolean())));
	declare_builtin_value("==", forall_a(fun({a, a}, boolean())));
	declare_builtin_value("!=", forall_a(fun({a, a}, boolean())));

	declare_builtin_value("&&", mono(fun({boolean(), boolean()}, boolean())));
	declare_builtin_value("||", mono(fun({boolean(), boolean()}, boolean())));

	declare_builtin_value("read_integer", mono(fun({}, integer())));
	declare_builtin_value("read_number",  mono(fun({}, number())));
	declare_builtin_value("read_string",  mono(fun({}, string())));
	declare_builtin_value("read_line",    mono(fun({}, string())));

	declare_builtin_typefunc("int",     BuiltinType::Int);
	declare_builtin_typefunc("float",   BuiltinType::Float);
	declare_builtin_typefunc("string",  BuiltinType::String);
	declare_builtin_typefunc("boolean", BuiltinType::Boolean);
	declare_builtin_typefunc("array",   BuiltinType::Array);
}

Type TypeChecker::new_var() {
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

Type TypeChecker::integer() {
	return Type(0);
}

Type TypeChecker::number() {
	return Type(1);
}

Type TypeChecker::string() {
	return Type(2);
}

Type TypeChecker::boolean() {
	return Type(3);
}

Type TypeChecker::unit() {
	return Type(4);
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
