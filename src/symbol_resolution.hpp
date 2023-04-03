#pragma once

#include "./utils/error_report.hpp"

namespace AST {
struct Expr;
struct Declaration;
struct Program;
}

namespace Frontend {

struct SymbolTable;
struct Context;

/*
 * Matches every identifier in the given ast with a declaration.
 * This also includes captures in a closure.
 */
[[nodiscard]] ErrorReport resolve_symbols(AST::Expr* ast, Context const& file_context, SymbolTable&);
[[nodiscard]] ErrorReport resolve_symbols_program(AST::Program* ast, Context const& file_context, SymbolTable&);

} // namespace Frontend
