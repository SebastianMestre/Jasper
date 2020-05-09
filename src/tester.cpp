#include "tester.hpp"

#include <string>
#include <vector>

#include "ast.hpp"
#include "eval.hpp"
#include "execute.hpp"
#include "garbage_collector.hpp"

namespace Test {

Parser make_parser(std::string source, Lexer& l) {
	std::vector<char> v;
	
	for (char c : source) {
		v.push_back(c);
	}

	l.m_source = std::move(v);

	Parser p;
	p.m_lexer = &l;

	return p;
}

Tester::Tester(std::string s) : m_source(std::move(s)) {};
Tester::Tester(std::string s, std::vector<TestFunction> tfs)
    : m_source(std::move(s)), m_testers(std::move(tfs)) {};

void Tester::add_test(TestFunction tf) {
	m_testers.push_back(tf);
}

bool Tester::execute(bool print_parse = false) {
	bool answer = true;
	::execute(m_source, print_parse, [&](Type::Environment& env) {
		for (auto* f : m_testers) {
			answer = answer and (*f)(env);
		}
	});
	return answer;
}

}
