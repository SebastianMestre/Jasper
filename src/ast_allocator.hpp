#pragma once

#include "ast.hpp"
#include "node_allocator.hpp"

namespace AST {

// TODO: could we use the AST_TAGS macro here?
struct Allocator : public NodeAllocator<
    NumberLiteral,
    IntegerLiteral,
    StringLiteral,
    BooleanLiteral,
    NullLiteral,
    ObjectLiteral,
    ArrayLiteral,
    DictionaryLiteral,
    FunctionLiteral,
    ShortFunctionLiteral,
    DeclarationList,
    Declaration,
    Identifier,
    BinaryExpression,
    CallExpression,
    IndexExpression,
    RecordAccessExpression,
    ConstructorExpression,
    Block,
    ReturnStatement,
    IfElseStatement,
    TernaryExpression,
    ForStatement,
    WhileStatement,
    TypeTerm,
    TypeVar,
    UnionExpression,
    TupleExpression,
    StructExpression> {};

} // namespace AST
