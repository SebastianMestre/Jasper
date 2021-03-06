#pragma once

#include "error_report.hpp"

namespace AST {
struct AST;
struct Declaration;
}

template<typename T>
struct ChunkedArray;

namespace Frontend {

/*
 * Matches every identifier in the given ast with a declaration.
 * This also includes captures in a closure.
 */
[[nodiscard]] ErrorReport match_identifiers(
    AST::AST* ast, ChunkedArray<AST::Declaration>&);

} // namespace Frontend
