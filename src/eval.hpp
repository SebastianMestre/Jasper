#pragma once

struct TypedAST;
namespace Type {
struct Environment;
struct Value;
}

Type::Value* eval(TypedAST*, Type::Environment&);

