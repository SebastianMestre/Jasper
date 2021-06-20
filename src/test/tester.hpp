#pragma once

#include <memory>
#include <string>
#include <vector>

#include "test.hpp"

namespace Test {

struct Tester {

	std::vector<Test> m_test_sets;

	Tester() = default;
	Tester(Test);
	Tester(std::vector<Test>);

	void add_test(Test);
	void add_tests(std::vector<Test>);
	TestReport execute();
};

} // namespace Test
