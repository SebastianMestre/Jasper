#pragma once

#include <vector>
#include <string>

#include "environment.hpp"
#include "parser.hpp"

namespace Test {

using TestFunction = bool (*)(Type::Environment&);

Parser make_parser(std::string source, Lexer& l);

struct Tester {

	std::string m_source;
	std::vector<TestFunction> m_testers;

	Tester(std::string);
	Tester(std::string, std::vector<TestFunction>);

	void add_test(TestFunction);
	bool execute(bool);
};

}
