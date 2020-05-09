#include "tester.hpp"

#include <string>
#include <vector>

#include "ast.hpp"
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

bool Tester::execute(bool print_parse = false) {
	int exit_code = ::execute(m_source, print_parse, [&](Type::Environment& env) -> int {
		for (auto* f : m_testers) {
			if (!(*f)(env))
				return 1;
		}
		return 0;
	});
	return exit_code == 0;
}

}
