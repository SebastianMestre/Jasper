#pragma once

#include <memory>

#include "parser.hpp"
#include "token_array.hpp"

Writer<std::unique_ptr<AST::AST>> parse_program(std::string const&, TokenArray&);
Writer<std::unique_ptr<AST::AST>> parse_expression(std::string const&, TokenArray&);
