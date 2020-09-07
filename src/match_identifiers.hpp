#pragma once

#include "error_report.hpp"

namespace TypedAST {
struct TypedAST;
}

namespace Frontend {
struct CompileTimeEnvironment;
}

namespace TypeChecker {

[[nodiscard]] ErrorReport match_identifiers(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment&);

}
