#pragma once

#include <vector>
#include <string>

#include "test_set.hpp"

namespace Test {

struct Tester {

	std::vector<TestSet> m_test_sets;

	Tester(TestSet);
	Tester(std::vector<TestSet>);

	void add_test(TestSet);
	void add_tests(const std::vector<TestSet>&);
	void execute(bool = false);
};

}
