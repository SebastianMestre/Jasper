#pragma once

struct AST;
namespace Type {
struct Environment;
struct Value;
}

Type::Value* eval(AST*, Type::Environment&);

