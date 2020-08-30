#pragma once

#include <vector>
#include <string>

#include "test_status.hpp"
#include "../interpreter/exit_status_type.hpp"
#include "../interpreter/environment_fwd.hpp"

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
