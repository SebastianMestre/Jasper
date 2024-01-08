#pragma once

namespace AST {
struct Program;
}

namespace TypeChecker {

struct TypeChecker;

void reify_types(AST::Program*, TypeChecker& tc);

} // namespace TypeChecker
