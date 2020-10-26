#pragma once

#include "ast.hpp"
// #include "utils/node_allocator.hpp"
#include "utils/automatic_block_allocator.hpp"

namespace AST {

/*
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
	*/

struct Allocator : public AutomaticBlockAllocator {
	Allocator() : AutomaticBlockAllocator(120, 4096) {}
};

} // namespace AST
