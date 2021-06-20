#include "tester.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../utils/typedefs.hpp"
#include "test_status_tag.hpp"

namespace Test {

Tester::Tester(TestSet ts) {
	m_test_sets.push_back(std::move(ts));
}

Tester::Tester(std::vector<TestSet> tss)
    : m_test_sets(std::move(tss)) {}

void Tester::add_test(TestSet ts) {
	m_test_sets.push_back(std::move(ts));
}

void Tester::add_tests(std::vector<TestSet> tss) {
	for (int i = 0; i < tss.size(); ++i)
		m_test_sets.push_back(std::move(tss[i]));
}

TestReport Tester::execute() {

	auto to_text = [](TestStatus status) -> char const* {
		switch (status) {
		case TestStatus::Ok:
			return ".";
		case TestStatus::Error:
			return "E";
		case TestStatus::Fail:
			return "F";
		case TestStatus::Empty:
			return "R";
		case TestStatus::MissingFile:
			return "?";
		default:
			return nullptr;
		}
	};

	auto veredict = TestStatus::Ok;
	std::vector<std::string> reports;

	for (int i = 0; i < m_test_sets.size(); ++i) {
		TestReport ts_answer = m_test_sets[i].execute();

		auto const& msg = ts_answer.m_msg;
		auto const& code = ts_answer.m_code;

		veredict = worst_of(veredict, code);
		auto status_text = to_text(code);
		if (status_text) {
			std::cout << status_text;
		} else {
			std::stringstream ss;
			ss << "Test number " << i + 1 << ": returned an invalid TestStatus (" << int(code) << ")";
			reports.push_back(ss.str());
		}

		if (!ts_answer.m_msg.empty()) {
			std::stringstream ss;
			ss << "Test number " << i + 1 << ": " << msg;
			reports.push_back(ss.str());
		}
	}

	std::cout << std::endl;
	for (const auto& r : reports)
		std::cout << r << std::endl;
	
	// TODO: put some text in the report
	return {veredict, ""};
}

} // namespace Test
