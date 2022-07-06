#pragma once

namespace CST {
struct CST;
}

namespace AST {

struct AST;
struct Allocator;

AST* convert_ast(CST::CST*, Allocator& alloc);

}
