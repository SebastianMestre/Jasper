#pragma once

#include "ast.hpp"
#include "node_allocator.hpp"

namespace AST {

// TODO: could we use the AST_TAGS macro here?
using Allocator = NodeAllocator<
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
    StructExpression>;

} // namespace AST
