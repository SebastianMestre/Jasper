#pragma once

#include "compile_time_environment.hpp"
#include "typesystem.hpp"
#include "utils/chunked_array.hpp"
#include "utils/interned_string.hpp"

namespace AST {
struct Allocator;
struct Declaration;
}

namespace TypeChecker {

struct TypeChecker {

	TypeSystemCore m_core;
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
	PolyId generalize(MonoId mono);
	void bind_free_vars(MonoId mono);
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);

	MetaTypeId new_meta_var();

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();
};

} // namespace TypeChecker
