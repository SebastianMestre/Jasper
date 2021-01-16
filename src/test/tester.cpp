#include "tester.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../utils/typedefs.hpp"
#include "test_status_tag.hpp"

namespace Test {

Tester::Tester(Own<TestSet> ts) {
	m_test_sets.push_back(std::move(ts));
}

Tester::Tester(std::vector<Own<TestSet>> tss)
    : m_test_sets(std::move(tss)) {}

void Tester::add_test(Own<TestSet> ts) {
	m_test_sets.push_back(std::move(ts));
}

void Tester::add_tests(std::vector<Own<TestSet>> tss) {
	for (int i = 0; i < tss.size(); ++i)
		m_test_sets.push_back(std::move(tss[i]));
}

TestReport Tester::execute() {
	std::vector<std::string> reports;

	auto veredict = TestStatusTag::Ok;

	for (int i = 0; i < m_test_sets.size(); ++i) {
		TestReport ts_answer = m_test_sets[i]->execute();

		switch (ts_answer.m_code) {
		case TestStatusTag::Ok:
			std::cout << '.';
			break;
		case TestStatusTag::Error:
			veredict = TestStatusTag::Error;
			std::cout << 'E';
			break;
		case TestStatusTag::Fail:
			if (veredict != TestStatusTag::Error)
				veredict = TestStatusTag::Fail;
			std::cout << 'F';
			break;
		case TestStatusTag::Empty:
			if (veredict != TestStatusTag::Error && veredict != TestStatusTag::Fail)
				veredict = TestStatusTag::Empty;
			std::cout << 'R';
			break;
		case TestStatusTag::MissingFile:
			std::cout << '?';
			break;
		default:
			std::cout << " -Unknown test code- ";
		}

		if (ts_answer.m_msg.size())
			reports.push_back(ts_answer.m_msg);
	}

	std::cout << std::endl;
	for (const auto& r : reports)
		std::cout << r << std::endl;
	
	// TODO: put some text in the report
	return {veredict, ""};
}

} // namespace Test
