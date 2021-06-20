#pragma once

#include <string>
#include <vector>

#include "../interpreter/exit_status_tag.hpp"
#include "test_status_tag.hpp"

namespace Interpreter {
struct Interpreter;
}

namespace Frontend {
struct SymbolTable;
}

namespace Test {

struct InterpreterTestSet {
	using Interpret = ExitStatus (*)(Interpreter::Interpreter&, Frontend::SymbolTable&);

	std::string m_source_file;
	std::vector<Interpret> m_testers;
	bool m_dump = false;

	InterpreterTestSet(std::string);
	InterpreterTestSet(std::string, Interpret);
	InterpreterTestSet(std::string, std::vector<Interpret>);

	TestReport execute();
};

struct NormalTestSet {
	using TestFunction = TestReport (*)();

	NormalTestSet();
	NormalTestSet(TestFunction);
	NormalTestSet(std::vector<TestFunction>);

	std::vector<TestFunction> m_testers;

	TestReport execute();
};

} // namespace Test
