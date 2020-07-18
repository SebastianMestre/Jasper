#include "execute.hpp"

#include "captures.hpp"
#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "match_identifiers.hpp"
#include "native.hpp"
#include "parse.hpp"
#include "token_array.hpp"
#include "typed_ast.hpp"

test_type execute(std::string const& source, bool dump_ast, Runner* runner) {

	TokenArray ta;
	auto parse_result = parse_program(source, ta);
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return test_type::ParseError;
	}

	if (dump_ast)
		print(parse_result.m_result.get(), 1);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = { &gc, &scope, &scope };

	auto top_level_ast = std::move(parse_result.m_result);
	if (top_level_ast->type() != ast_type::DeclarationList)
		return test_type::TopLevelTypeError;

	declare_native_functions(env);

	auto top_level = TypedAST::get_unique(top_level_ast);
	TypeChecker::match_identifiers(top_level.get());
	gather_captures(top_level.get());
	eval(top_level.get(), env);

	test_type runner_exit_code = runner(env);

	if(test_type::Ok != runner_exit_code)
		return runner_exit_code;

	gc.run();

	return test_type::Ok;
}

// NOTE: We currently implement funcion evaluation in eval(AST::CallExpression)
// this means we need to create a call expression node to run the program.
// TODO: We need to clean this up
Type::Value* eval_expression(const std::string& expr, Type::Environment& env) {
	TokenArray ta;

	auto top_level_call_ast = parse_expression(expr, ta);
	auto top_level_call = TypedAST::get_unique(top_level_call_ast.m_result);

	return Type::unboxed(eval(top_level_call.get(), env));
}
