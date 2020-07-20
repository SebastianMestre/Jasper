#pragma once

#include <vector>
#include <string>

#include "exit_status_type.hpp"
#include "environment_fwd.hpp"

namespace Test {

using TestFunction = exit_status_type (*)(Type::Environment&);

struct TestSet {

	std::string m_source;
	std::vector<TestFunction> m_testers;

	TestSet(std::string);
	TestSet(std::string, TestFunction);
	TestSet(std::string, std::vector<TestFunction>);

	exit_status_type execute(bool);
};

}
