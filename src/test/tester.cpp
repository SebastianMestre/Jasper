#include "tester.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "test_status.hpp"

namespace Test {

template <typename T>
using Own = std::unique_ptr<T>;

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

void Tester::execute() {
	std::vector<std::string> reports;

	for (int i = 0; i < m_test_sets.size(); ++i) {
		TestReport ts_answer = m_test_sets[i]->execute();

		switch (ts_answer.m_code) {
		case test_status::Ok:
			std::cout << '.';
			break;
		case test_status::Error:
			std::cout << 'E';
			break;
		case test_status::Fail:
			std::cout << 'F';
			break;
		case test_status::Empty:
			std::cout << 'R';
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
}

} // namespace Test
