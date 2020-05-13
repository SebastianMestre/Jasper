#include "execute.hpp"

#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "parse.hpp"
#include "captures.hpp"

int execute(std::string const& source, bool dump_ast, Runner* runner) {

	auto parse_result = parse_program(source);
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return 1;
	}

	gather_captures(parse_result.m_result.get());

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
