#pragma once

#include <string>

#include "parser.hpp"
#include "token_array.hpp"

namespace AST {
struct Allocator;
}

Writer<AST::AST*> parse_program(std::string const&, TokenArray&, AST::Allocator&);
Writer<AST::AST*> parse_expression(std::string const&, TokenArray&, AST::Allocator&);
