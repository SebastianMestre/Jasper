#pragma once

#include <memory>

#include "ast_allocator.hpp"

namespace AST {

AST* desugar(AST*, Allocator&);

} // namespace AST
