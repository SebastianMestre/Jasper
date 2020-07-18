#pragma once

#include <vector>
#include <string>

#include "test_type.hpp"
#include "environment_fwd.hpp"

namespace Test {

using TestFunction = test_type (*)(Type::Environment&);

struct TestSet {

	std::string m_source;
	std::vector<TestFunction> m_testers;

	TestSet(std::string);
	TestSet(std::string, TestFunction);
	TestSet(std::string, std::vector<TestFunction>);

	test_type execute(bool);
};

}
