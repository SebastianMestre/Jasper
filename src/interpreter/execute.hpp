#pragma once

#include "environment_fwd.hpp"
#include "exit_status_type.hpp"
#include "value.hpp"
#include <string>

namespace Interpreter {

using Runner = auto(Environment&) -> ExitStatusTag;

// returns an exit status
ExitStatusTag execute(std::string const& source, bool dump_ast, Runner* runner);

// creates an expression node and returns the unboxed value from it
Value* eval_expression(const std::string& expr, Environment& env);

} // namespace Interpreter
