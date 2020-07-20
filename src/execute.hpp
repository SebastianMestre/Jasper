#pragma once

#include <string>
#include "environment_fwd.hpp"
#include "exit_status_type.hpp"
#include "value.hpp"

using Runner = auto (Type::Environment&) -> exit_status_type;

// returns an exit status
exit_status_type execute(std::string const& source, bool dump_ast, Runner* runner);

// creates an expression node and returns the unboxed value from it
Type::Value* eval_expression(const std::string& expr, Type::Environment& env);
