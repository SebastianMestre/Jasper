#pragma once

#include <string>

#include "./lexer_result.hpp"
#include "./parser_result.hpp"

namespace CST {
struct Allocator;
struct Program;
struct Expr;
}

ParserResult<CST::Program> parse_program(LexerResult, CST::Allocator&);
ParserResult<CST::Expr> parse_expression(LexerResult, CST::Allocator&);
