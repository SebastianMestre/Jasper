#pragma once

#include "../utils/chunked_array.hpp"
#include "../utils/interned_string.hpp"
#include "core.hpp"

namespace AST {
struct Allocator;
struct Declaration;
struct Program;
}

namespace TypeChecker {

struct TypeChecker {

	ChunkedArray<AST::Declaration> m_builtin_declarations;

	AST::Allocator* m_ast_allocator;
	bool m_in_last_metacheck_pass {false};

	TypeChecker(AST::Allocator& allocator);

	AST::Declaration* new_builtin_declaration(InternedString const& name);
	void declare_builtin_value(InternedString const& name, PolyId);
	void declare_builtin_typefunc(InternedString const& name, TypeFunc);

	Type new_var();

	Type mono_int();
	Type mono_float();
	Type mono_string();
	Type mono_boolean();
	Type mono_unit();

	TypeSystemCore& core() { return m_core; }

	std::vector<std::vector<AST::Declaration*>> const& declaration_order() const;
	void compute_declaration_order(AST::Program* ast);

private:
	TypeSystemCore m_core;
	std::vector<std::vector<AST::Declaration*>> m_declaration_components;
};

} // namespace TypeChecker
