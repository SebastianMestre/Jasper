#include "tester.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "test_type.hpp"

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

void Tester::execute(bool dump_ast) {
	// TODO: dump ast to file to prevent
	// unreadable test output
	std::vector<test_type> errors;

	for (auto ts : m_test_sets) {
		test_type ts_answer = ts.execute(dump_ast);
		switch(ts_answer){
		case test_type::Ok:
			std::cout << '.';
			break;
		case test_type::ParseError:
		case test_type::TopLevelTypeError:
			errors.push_back(ts_answer);
			std::cout << 'E';
			break;
		case test_type::NullResult:
		case test_type::TypeError:
		case test_type::ValueError:
			errors.push_back(ts_answer);
			std::cout << 'F';
			break;
		case test_type::Empty:
			errors.push_back(ts_answer);
			std::cout << 'R';
			break;
		default:
			std::cout << " -Unknown test code- ";
		}
	}
	std::cout << std::endl;

	for(auto tt : errors)
		std::cout << test_type_string[static_cast<int>(tt)] << std::endl;
}

}
