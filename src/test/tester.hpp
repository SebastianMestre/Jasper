#pragma once

#include <memory>
#include <string>
#include <vector>

#include "test.hpp"
#include "test_set.hpp"

namespace Test {

struct Tester {

	std::vector<TestSet> m_test_sets;

	Tester() = default;
	Tester(TestSet);
	Tester(std::vector<TestSet>);

	void add_test(TestSet);
	void add_tests(std::vector<TestSet>);
	TestReport execute();
};

} // namespace Test
