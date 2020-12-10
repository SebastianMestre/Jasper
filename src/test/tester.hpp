#pragma once

#include <memory>
#include <string>
#include <vector>

#include "test_set.hpp"

namespace Test {

struct Tester {

	std::vector<std::unique_ptr<TestSet>> m_test_sets;

	Tester() = default;
	Tester(std::unique_ptr<TestSet>);
	Tester(std::vector<std::unique_ptr<TestSet>>);

	void add_test(std::unique_ptr<TestSet>);
	void add_tests(std::vector<std::unique_ptr<TestSet>>);
	TestReport execute();
};

} // namespace Test
