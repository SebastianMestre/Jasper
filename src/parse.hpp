#pragma once

#include <memory>

#include "parser.hpp"

Writer<std::unique_ptr<AST>> parse_program(std::string const&);
Writer<std::unique_ptr<AST>> parse_expression(std::string const&);
