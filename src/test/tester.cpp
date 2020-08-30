#include "tester.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "test_status.hpp"

namespace Test {

Tester::Tester(TestSet ts) : m_test_sets({ std::move(ts) }) {};
Tester::Tester(std::vector<TestSet> tss) : m_test_sets(std::move(tss)) {};

void Tester::add_test(TestSet ts) {
	m_test_sets.push_back(ts);
}

void Tester::add_tests(const std::vector<TestSet>& tss) {
	for (auto ts : tss)
		m_test_sets.push_back(ts);
}

void Tester::execute() {
	std::vector<std::string> reports;

	for (auto ts : m_test_sets) {
		TestReport ts_answer = ts.execute();
		
		switch(ts_answer.m_code){
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
	for(const auto& r : reports)
		std::cout << r << std::endl;
}

}
