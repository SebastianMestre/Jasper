#pragma once

#include <vector>
#include <string>

#include "environment.hpp"

using TestFunction = void (*)(Type::Environment);

struct Tester {

	std::string m_source;
	std::vector<TestFunction> m_testers;

	Tester(std::string);
	Tester(std::string, std::vector<TestFunction>);

	void add_test(TestFunction);
};
