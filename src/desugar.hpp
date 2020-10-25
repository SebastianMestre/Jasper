#pragma once

namespace AST {

struct AST;
struct Allocator;

AST* desugar(AST*, Allocator&);

} // namespace AST
