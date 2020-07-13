#pragma once

#include <memory>

namespace AST {
struct AST;

// Takes ownership of the given ast.
// This lets us re-use a large amount of the nodes, working a lot faster.
std::unique_ptr<AST> desugar(std::unique_ptr<AST>);

}
