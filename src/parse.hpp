#pragma once

#include <string>

#include "parser.hpp"
#include "token_array.hpp"

namespace CST {
struct Allocator;
}

Writer<CST::CST*> parse_program(std::string const&, TokenArray&, CST::Allocator&);
Writer<CST::CST*> parse_expression(std::string const&, TokenArray&, CST::Allocator&);
