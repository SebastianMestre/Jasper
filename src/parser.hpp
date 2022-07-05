#pragma once

#include <string>

#include "./utils/writer.hpp"

namespace CST {
struct CST;
struct Allocator;
}

struct LexerResult;

Writer<CST::CST*> parse_program(LexerResult const&, CST::Allocator&);
Writer<CST::CST*> parse_expression(LexerResult const&, CST::Allocator&);
