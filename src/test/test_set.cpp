#include "test_set.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../utils/typedefs.hpp"
#include "test_report.hpp"

namespace Test {

TestSet::TestSet(Test ts) {
	m_test_sets.push_back(std::move(ts));
}

TestSet::TestSet(std::vector<Test> tss)
    : m_test_sets(std::move(tss)) {}

void TestSet::add_test(Test ts) {
	m_test_sets.push_back(std::move(ts));
}

void TestSet::add_tests(std::vector<Test> tss) {
	for (int i = 0; i < tss.size(); ++i)
		m_test_sets.push_back(std::move(tss[i]));
}

TestReport TestSet::execute() {

	auto veredict = TestStatus::Ok;
	std::vector<std::string> reports;

	TestReport full_report;

	for (int i = 0; i < m_test_sets.size(); ++i) {
		TestReport report = m_test_sets[i].execute();

		std::stringstream prefix;
		prefix << "[" << i << "] ";
		auto prefix_str = prefix.str();

		for (auto& report_piece : report.m_pieces) {
			if (!report_piece.m_message.empty()) {
				report_piece.m_message.insert(
				    report_piece.m_message.begin(),
				    prefix_str.begin(),
				    prefix_str.end());
			}
		}

		full_report += report;
	}

	return full_report;
}

} // namespace Test
