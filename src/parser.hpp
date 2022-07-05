#pragma once

#include <string>

#include "./parser_result.hpp"

namespace CST {
struct CST;
struct Allocator;
}

struct LexerResult;

ParserResult parse_program(LexerResult const&, CST::Allocator&);
ParserResult parse_expression(LexerResult const&, CST::Allocator&);
