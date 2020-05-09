#pragma once

#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "lexer.hpp"
#include "parser.hpp"

// returns an exit status
template <typename Runner>
int execute(std::string const& source, bool dump_ast, Runner runner) {
	static_assert(
	    std::is_same<decltype(runner(std::declval<Type::Environment&>())), int>::value,
	    "runners must have signature Environment& -> int");

	Lexer l;

	std::vector<char> v;
	for (char c : source)
		v.push_back(c);

	l.m_source = std::move(v);
	Parser p;
	p.m_lexer = &l;

	auto parse_result = p.parse_top_level();
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return 1;
	}

	if (dump_ast)
		print(parse_result.m_result.get(), 1);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = { &gc, &scope };

	auto* top_level = parse_result.m_result.get();
	if (top_level->type() != ast_type::DeclarationList)
		return 1;

	eval(top_level, env);

	int runner_exit_code = runner(env);

	if(runner_exit_code)
		return runner_exit_code;

	gc.run();

	return 0;
}
