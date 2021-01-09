#pragma once

#include "exit_status_tag.hpp"
#include <string>

namespace Interpreter {

struct Interpreter;
struct Value;

using Runner = auto(Interpreter&) -> ExitStatusTag;

// returns an exit status
ExitStatusTag execute(std::string const& source, bool dump_ast, Runner* runner);

// evaluates an expression and returns the resulting value
Value* eval_expression(const std::string& expr, Interpreter& env);

} // namespace Interpreter
