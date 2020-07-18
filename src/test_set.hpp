#pragma once

#include <vector>
#include <string>

#include "test_type.hpp"
#include "environment.hpp"

namespace Test {

using TestFunction = test_type (*)(Type::Environment&);

struct TestSet {

	std::string m_source;
	std::vector<TestFunction> m_testers;

	Tester(std::string);
	Tester(std::string, TestFunction);
	Tester(std::string, std::vector<TestFunction>);

	test_type execute(Type::Environment& env);
};

}
