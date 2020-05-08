#include "tester.hpp"

#include <string>

Tester::Tester(std::string s) : m_source(std::move(s)) {};
Tester::Tester(std::string s, std::vector<TestFunction> tfs)
	: m_source(std::move(s)), m_testers(std::move(tfs)) {};

void Tester::add_test(TestFunction tf) {
	m_testers.push_back(tf);
}
