#include <iostream>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

int main() {
	std::vector<char> v;
	std::string s = R"(
	myfoo : int = 123123;
	x : dec = 1.4;
	y : int = 3;
	z := fn () {};

)";
	for (char c : s) {
		v.push_back(c);
	}

	Lexer l;
	l.m_source = std::move(v);

	Parser p;
	p.m_lexer = &l;

	std::cout << s << '\n';

	auto parse_result = p.parse_top_level();
	if (not parse_result.ok()) {
		parse_result.m_error.print();
	} else {
		parse_result.m_result->print();
	}

	return 0;
}
