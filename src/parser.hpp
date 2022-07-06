#pragma once

#include <string>

#include "./lexer_result.hpp"
#include "./parser_result.hpp"

namespace CST {
struct Allocator;
}

ParserResult parse_program(LexerResult, CST::Allocator&);
ParserResult parse_expression(LexerResult, CST::Allocator&);
