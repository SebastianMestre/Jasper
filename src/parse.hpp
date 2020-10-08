#pragma once

#include <memory>
#include <utility>

#include "parser.hpp"
#include "token_array.hpp"
#include "ast_allocator.hpp"

Writer<AST::AST*> parse_program(std::string const&, TokenArray&, AST::Allocator&);
Writer<AST::AST*> parse_expression(std::string const&, TokenArray&, AST::Allocator&);
