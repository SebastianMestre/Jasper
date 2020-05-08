#include "tester.hpp"

#include <string>
#include <vector>

#include "ast.hpp"
#include "garbage_collector.hpp"
#include "eval.hpp"

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

	Lexer l;
	// intentionally copy, to preserve m_source
	Parser p = make_parser(m_source, l);

	auto parse_result = p.parse_top_level();
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return false;
	}

	if (print_parse)
		print(parse_result.m_result.get(), 1);

	auto* top_level = parse_result.m_result.get();
	assert(top_level->type() == ast_type::DeclarationList);
	
	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = {&gc, &scope};

	eval(top_level, env);

	bool answer = true;
	for (auto *f : m_testers) {
		answer = answer and (*f)(env);
	}

	gc.run();
	return answer;
}
