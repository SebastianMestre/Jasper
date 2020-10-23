#pragma once

#include "compile_time_environment.hpp"
#include "typed_ast_allocator.hpp"
#include "typesystem.hpp"
#include "utils/chunked_array.hpp"
#include "utils/interned_string.hpp"

namespace TypeChecker {

struct TypeChecker {

	TypeSystemCore m_core;
	Frontend::CompileTimeEnvironment m_env;
	ChunkedArray<TypedAST::Declaration> m_builtin_declarations;

	TypedAST::Allocator* m_typed_ast_allocator;

	TypeChecker(TypedAST::Allocator& allocator);

	void declare_builtin(InternedString const& name, MetaTypeId, PolyId);

	MonoId new_hidden_var();
	MonoId new_var();
	PolyId generalize(MonoId mono);
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);

	MetaTypeId new_meta_var();

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

	MetaTypeId meta_value();
	MetaTypeId meta_typefunc();
	MetaTypeId meta_monotype();
};

} // namespace TypeChecker
