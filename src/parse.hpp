#pragma once

#include <memory>
#include <utility>

#include "parser.hpp"
#include "token_array.hpp"

Writer<std::pair<AST::AST*, AST::Allocator>> parse_program(std::string const&, TokenArray&);
Writer<std::pair<AST::AST*, AST::Allocator>> parse_expression(std::string const&, TokenArray&);
