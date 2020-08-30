#pragma once

#include <vector>
#include <string>

#include "test_status.hpp"
#include "../interpreter/exit_status_type.hpp"
#include "../interpreter/environment_fwd.hpp"

namespace Test {

using Run = TestReport (*)();

struct TestSet {

	std::vector<Run> m_runners;

	TestSet() = default;
	TestSet(Run);
	TestSet(std::vector<Run>);

	virtual TestReport execute();
	virtual ~TestSet() = default;
};

using Interpret = exit_status_type (*)(Interpreter::Environment&);

struct TestInterpreter : public TestSet {

	std::string m_source;
	std::vector<Interpret> m_testers;
	bool m_dump = false;

	TestInterpreter(std::string);
	TestInterpreter(std::string, Interpret);
	TestInterpreter(std::string, std::vector<Interpret>);

	TestReport execute() override;
};

}
