#include "tester.hpp"

#include <string>
#include <vector>

#include "eval.hpp"
#include "execute.hpp"
#include "garbage_collector.hpp"

namespace Test {

Tester::Tester(std::string s) : m_source(std::move(s)) {};
Tester::Tester(std::string s, std::vector<TestFunction> tfs)
    : m_source(std::move(s)), m_testers(std::move(tfs)) {};

void Tester::add_test(TestFunction tf) {
	m_testers.push_back(tf);
}

void Tester::add_tests(const std::vector<TestFunction>& tfs) {
	for (auto* tf : tfs)
		m_testers.push_back(tf);
}

int Tester::execute(bool print_parse) {
	for (auto* tester : m_testers) {
		int exit_code = ::execute(m_source, print_parse, tester);
		if(exit_code != 0) return exit_code;
	}
	return 0;
}

}
