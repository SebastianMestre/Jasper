#pragma once

#include <cassert>

#include "value.hpp"

namespace Interpreter {

void eval_call_callable(Value callee, int arg_count, Interpreter&);

} // namespace Interpreter
