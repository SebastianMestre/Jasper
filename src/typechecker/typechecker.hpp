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
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);

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
};

} // namespace TypeChecker
