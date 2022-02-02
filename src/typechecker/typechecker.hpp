#pragma once

#include "../utils/chunked_array.hpp"
#include "../utils/interned_string.hpp"
#include "compile_time_environment.hpp"
#include "typesystem.hpp"

namespace AST {
struct Allocator;
struct Declaration;
struct Program;
}

namespace TypeChecker {

struct TypeChecker {

	Frontend::CompileTimeEnvironment m_env;
	ChunkedArray<AST::Declaration> m_builtin_declarations;

	AST::Allocator* m_ast_allocator;
	bool m_in_last_metacheck_pass {false};

	TypeChecker(AST::Allocator& allocator);

	AST::Declaration* new_builtin_declaration(InternedString const& name);
	void declare_builtin_value(InternedString const& name, PolyId);
	void declare_builtin_typefunc(InternedString const& name, TypeFunctionId);

	MonoId new_hidden_var();
	MonoId new_var();

	MetaTypeId new_meta_var();

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

	TypeSystemCore& core() { return m_core; }

	std::vector<std::vector<AST::Declaration*>> const& declaration_order() const;
	void compute_declaration_order(AST::Program* ast);

	Frontend::CompileTimeEnvironment& env() { return m_env; }
private:
	TypeSystemCore m_core;
	std::vector<std::vector<AST::Declaration*>> m_declaration_components;
};

struct Facade1 {
	TypeChecker& tc;

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

	void bind_free_vars(MonoId mono);
	PolyId generalize(MonoId mono);

	TypeSystemCore& core();
	std::vector<std::vector<AST::Declaration*>> const& declaration_order() const;

	MonoId new_hidden_var();
	MonoId new_var();

	void new_nested_scope();
	void end_scope();
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);
	Frontend::CompileTimeEnvironment& env();

	void unify(MonoId i, MonoId j) { core().m_mono_core.unify(i, j); }

	bool is_type(MetaTypeId i) {
		return meta_type_is(i, Tag::Func) || meta_type_is(i, Tag::Mono);
	}

	bool is_term(MetaTypeId i) {
		return meta_type_is(i, Tag::Term);
	}

	MonoId inst_fresh(PolyId i) {
		return tc.core().inst_fresh(i);
	}

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> arguments,
	    char const* debug_data = nullptr) {
		return core().new_term(type_function, std::move(arguments), debug_data);
	}

private:
	bool meta_type_is(MetaTypeId i, Tag t) {
		i = get_resolved_meta_type(i);
		return core().m_meta_core.is(i, t);
	}

	MetaTypeId get_resolved_meta_type(MetaTypeId i) {
		return core().m_meta_core.eval(i);
	}
};

} // namespace TypeChecker
