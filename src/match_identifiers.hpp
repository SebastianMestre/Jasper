#pragma once

#include "error_report.hpp"

namespace TypedAST {
struct TypedAST;
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
    TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment&);
} // namespace TypeChecker
