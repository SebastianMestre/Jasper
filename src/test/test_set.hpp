#pragma once

#include <string>
#include <vector>

#include "../interpreter/environment_fwd.hpp"
#include "../interpreter/exit_status_type.hpp"
#include "test_status.hpp"

namespace Test {

struct TestSet {

	virtual TestReport execute() = 0;
	virtual ~TestSet() = default;
};

using Interpret = exit_status_type (*)(Interpreter::Environment&);

struct InterpreterTestSet : public TestSet {

	std::string m_source;
	std::vector<Interpret> m_testers;
	bool m_dump = false;

	InterpreterTestSet(std::string);
	InterpreterTestSet(std::string, Interpret);
	InterpreterTestSet(std::string, std::vector<Interpret>);

	TestReport execute() override;
};

struct NormalTestSet : public TestSet {
	using TestFunction = TestReport (*)();

	NormalTestSet();
	NormalTestSet(std::vector<TestFunction>);

	std::vector<TestFunction> m_testers;

	TestReport execute() override;
};

}
