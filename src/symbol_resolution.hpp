#pragma once

#include "./utils/error_report.hpp"

namespace AST {
struct AST;
struct Declaration;
}

namespace Frontend {

struct SymbolTable;
struct Context;

/*
 * Matches every identifier in the given ast with a declaration.
 * This also includes captures in a closure.
 */
[[nodiscard]] ErrorReport resolve_symbols(AST::AST* ast, Context const& file_context, SymbolTable&);

} // namespace Frontend
