#pragma once

#include "error_report.hpp"

namespace AST {
struct AST;
struct Declaration;
}

namespace Frontend {

struct SymbolTable;

/*
 * Matches every identifier in the given ast with a declaration.
 * This also includes captures in a closure.
 */
[[nodiscard]] ErrorReport match_identifiers(AST::AST* ast, SymbolTable&);

} // namespace Frontend
