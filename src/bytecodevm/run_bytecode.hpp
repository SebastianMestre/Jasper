#pragma once

#include <vector>

namespace Interpreter {
struct Interpreter;
}

struct Instruction;

struct BytecodeRunner {
	Interpreter::Interpreter& e;
	BytecodeRunner(Interpreter::Interpreter& e_)
	    : e {e_} {}

	int run(std::vector<Instruction> const& instructions);
	int run_single(Instruction const& instruction);
};
