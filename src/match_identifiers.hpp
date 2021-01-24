#pragma once

#include "error_report.hpp"

namespace AST {
struct AST;
}

namespace Frontend {
struct CompileTimeEnvironment;
}

namespace TypeChecker {

/*
 * Matches every identifier in the given ast with a declaration.
 * This also includes captures in a closure.
 */
[[nodiscard]] ErrorReport match_identifiers(
    AST::AST* ast, Frontend::CompileTimeEnvironment&);
} // namespace TypeChecker
