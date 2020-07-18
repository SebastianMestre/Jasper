#pragma once

#include <string>
#include "environment_fwd.hpp"
#include "test_type.hpp"
#include "value.hpp"

using Runner = auto (Type::Environment&) -> test_type;

// returns an exit status
test_type execute(std::string const& source, bool dump_ast, Runner* runner);

// creates an expression node and returns the unboxed value from it
Type::Value* eval_expression(const std::string& expr, Type::Environment& env);
