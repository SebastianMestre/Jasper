#include "execute.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../cst_allocator.hpp"
#include "../ct_eval.hpp"
#include "../match_identifiers.hpp"
#include "../metacheck.hpp"
#include "../parse.hpp"
#include "../token_array.hpp"
#include "../typecheck.hpp"
#include "../typechecker.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "native.hpp"
#include "utils.hpp"

namespace Interpreter {

ExitStatusTag execute(std::string const& source, bool dump_cst, Runner* runner) {

	TokenArray ta;
	CST::Allocator cst_allocator;
	auto parse_result = parse_program(source, ta, cst_allocator);

	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return ExitStatusTag::ParseError;
	}

	auto cst = parse_result.m_result;

	if (dump_cst)
		print(cst, 1);

	// Can this even happen? parse_program should always either return a
	// DeclarationList or an error
	if (cst->type() != CSTTag::DeclarationList)
		return ExitStatusTag::TopLevelTypeError;

	AST::Allocator ast_allocator;
	auto ast = AST::convert_ast(cst, ast_allocator);
	TypeChecker::TypeChecker tc{ast_allocator};

	{
		auto err = TypeChecker::match_identifiers(ast, tc.m_env);
		if (!err.ok()) {
			err.print();
			return ExitStatusTag::StaticError;
		}
	}
	tc.m_env.compute_declaration_order(static_cast<AST::DeclarationList*>(ast));

	TypeChecker::metacheck(ast, tc);
	ast = TypeChecker::ct_eval(ast, tc, ast_allocator);
	TypeChecker::typecheck(ast, tc);
	TypeChecker::compute_offsets(ast, 0);

	GC gc;
	Interpreter env = {&tc, &gc};
	declare_native_functions(env);
	eval(ast, env);

	ExitStatusTag runner_exit_code = runner(env);

	return runner_exit_code;
}

// NOTE: We currently implement funcion evaluation in eval(CST::CallExpression)
// this means we need to create a call expression node to run the program.
// Having the TokenArray die here is not good, as it takes ownership of the tokens
// TODO: We need to clean this up
Value* eval_expression(const std::string& expr, Interpreter& env) {
	TokenArray ta;
	CST::Allocator cst_allocator;
	AST::Allocator ast_allocator;

	auto parse_result = parse_expression(expr, ta, cst_allocator);
	auto cst = parse_result.m_result;
	auto ast = AST::convert_ast(cst, ast_allocator);

	// TODO: return a gc_ptr
	eval(ast, env);
	auto value = env.m_stack.pop();
	return value_of(value.get());
}

} // namespace Interpreter
