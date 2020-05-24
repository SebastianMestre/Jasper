#pragma once

#include <string>
#include "environment_fwd.hpp"

using Runner = auto (Type::Environment&) -> int;

// returns an exit status
int execute(std::string const& source, bool dump_ast, Runner* runner);
