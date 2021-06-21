#pragma once

#include <memory>
#include <string>
#include <vector>

#include "test.hpp"

namespace Test {

struct TestSet {

	std::vector<Test> m_test_sets;

	TestSet() = default;
	TestSet(Test);
	TestSet(std::vector<Test>);

	void add_test(Test);
	void add_tests(std::vector<Test>);
	TestReport execute();
};

} // namespace Test
