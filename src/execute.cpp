#include "execute.hpp"

#include "captures.hpp"
#include "desugar.hpp"
#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "match_identifiers.hpp"
#include "native.hpp"
#include "parse.hpp"
#include "token_array.hpp"
#include "typed_ast.hpp"

int execute(std::string const& source, bool dump_ast, Runner* runner) {

	TokenArray ta;
	auto parse_result = parse_program(source, ta);
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return 1;
	}


	if (dump_ast)
		print(parse_result.m_result.get(), 1);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = { &gc, &scope, &scope};

	auto top_level_ast = std::move(parse_result.m_result);
	if (top_level_ast->type() != ast_type::DeclarationList)
		return 1;

	auto desugared_ast = AST::desugar(std::move(top_level_ast));

	declare_native_functions(env);

	auto top_level = TypedAST::get_unique(desugared_ast);
	TypeChecker::match_identifiers(top_level.get());
	gather_captures(top_level.get());
	eval(top_level.get(), env);

	int runner_exit_code = runner(env);

	if(runner_exit_code)
		return runner_exit_code;

	gc.run();

	return 0;
}
