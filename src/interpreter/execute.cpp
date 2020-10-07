#include "execute.hpp"

#include "../ct_eval.hpp"
#include "../desugar.hpp"
#include "../match_identifiers.hpp"
#include "../metacheck.hpp"
#include "../parse.hpp"
#include "../token_array.hpp"
#include "../typecheck.hpp"
#include "../typechecker.hpp"
#include "../typed_ast.hpp"
#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "native.hpp"
#include "utils.hpp"

#include <iostream>

namespace Interpreter {

ExitStatusTag execute(std::string const& source, bool dump_ast, Runner* runner) {

	TokenArray ta;
	AST::Allocator ast_allocator;
	auto parse_result = parse_program(source, ta, ast_allocator);

	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return ExitStatusTag::ParseError;
	}

	if (dump_ast)
		print(parse_result.m_result, 1);

	// Can this even happen? parse_program should always either return a
	// DeclarationList or an error
	if (parse_result.m_result->type() != ASTTag::DeclarationList)
		return ExitStatusTag::TopLevelTypeError;

	auto desugared_ast = AST::desugar(parse_result.m_result, ast_allocator);
	auto top_level = TypedAST::convert_ast(desugared_ast);
	TypeChecker::TypeChecker tc;

	{
		auto err = TypeChecker::match_identifiers(top_level.get(), tc.m_env);
		if (!err.ok()) {
			err.print();
			return ExitStatusTag::StaticError;
		}
	}

	TypeChecker::metacheck(top_level.get(), tc);
	top_level = TypeChecker::ct_eval(std::move(top_level), tc);
	TypeChecker::typecheck(top_level.get(), tc);

	GC gc;
	Environment env = {&gc};
	declare_native_functions(env);
	eval(top_level.get(), env);

	ExitStatusTag runner_exit_code = runner(env);

	return runner_exit_code;
}

// NOTE: We currently implement funcion evaluation in eval(AST::CallExpression)
// this means we need to create a call expression node to run the program.
// Having the TokenArray die here is not good, as it takes ownership of the tokens
// TODO: We need to clean this up
Value* eval_expression(const std::string& expr, Environment& env) {
	TokenArray ta;
	AST::Allocator ast_allocator;

	auto top_level_call_ast = parse_expression(expr, ta, ast_allocator);
	auto top_level_call = TypedAST::convert_ast(top_level_call_ast.m_result);

	// TODO: return a gc_ptr
	auto value = eval(top_level_call.get(), env);
	return unboxed(value.get());
}

} // namespace Interpreter
