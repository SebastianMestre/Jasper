#include "execute.hpp"

#include "captures.hpp"
#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "parse.hpp"
#include "token_array.hpp"
#include "native.hpp"
#include "typed_ast.hpp"

int execute(std::string const& source, bool dump_ast, Runner* runner) {

	TokenArray ta;
	auto parse_result = parse_program(source, ta);
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return 1;
	}

	gather_captures(parse_result.m_result.get());

	if (dump_ast)
		print(parse_result.m_result.get(), 1);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = { &gc, &scope, &scope};

	auto* top_level_ast = parse_result.m_result.get();
	if (top_level_ast->type() != ast_type::DeclarationList)
		return 1;

	declare_native_functions(env);

	auto top_level = convertAST(top_level_ast);
	eval(top_level, env);

	int runner_exit_code = runner(env);

	if(runner_exit_code)
		return runner_exit_code;

	gc.run();

	return 0;
}
