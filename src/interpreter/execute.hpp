#pragma once

#include "exit_status_tag.hpp"
#include "value.hpp"
#include <string>

namespace Frontend {
struct SymbolTable;
}

namespace Interpreter {

struct Interpreter;

using Runner = auto(Interpreter&, Frontend::SymbolTable&) -> ExitStatus;

struct ExecuteSettings {
	bool dump_cst {false};
	bool typecheck {true};
};

// returns an exit status
ExitStatus execute(
	std::string const& source,
	ExecuteSettings settings,
	Runner* runner
);

// evaluates an expression and returns the resulting value
Value eval_expression(
	const std::string& expr,
	Interpreter& env,
	Frontend::SymbolTable&
);

} // namespace Interpreter
