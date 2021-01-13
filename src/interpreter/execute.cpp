#include "execute.hpp"

#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../ct_eval.hpp"
#include "../desugar.hpp"
#include "../match_identifiers.hpp"
#include "../metacheck.hpp"
#include "../parse.hpp"
#include "../token_array.hpp"
#include "../typecheck.hpp"
#include "../typechecker.hpp"
#include "../typed_ast.hpp"
#include "../typed_ast_allocator.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "native.hpp"
#include "utils.hpp"

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

	TypedAST::Allocator typed_ast_allocator;
	auto top_level = TypedAST::convert_ast(desugared_ast, typed_ast_allocator);
	TypeChecker::TypeChecker tc{typed_ast_allocator};

	{
		auto err = TypeChecker::match_identifiers(top_level, tc.m_env);
		if (!err.ok()) {
			err.print();
			return ExitStatusTag::StaticError;
		}
	}
	tc.m_env.compute_declaration_order(static_cast<TypedAST::DeclarationList*>(top_level));

	TypeChecker::metacheck(top_level, tc);
	top_level = TypeChecker::ct_eval(top_level, tc, typed_ast_allocator);
	TypeChecker::typecheck(top_level, tc);
	TypeChecker::compute_offsets(top_level, 0);

	GC gc;
	Interpreter env = {&tc, &gc};
	declare_native_functions(env);
	eval(top_level, env);

	ExitStatusTag runner_exit_code = runner(env);

	return runner_exit_code;
}

// NOTE: We currently implement funcion evaluation in eval(AST::CallExpression)
// this means we need to create a call expression node to run the program.
// Having the TokenArray die here is not good, as it takes ownership of the tokens
// TODO: We need to clean this up
Value* eval_expression(const std::string& expr, Interpreter& env) {
	TokenArray ta;
	AST::Allocator ast_allocator;
	TypedAST::Allocator typed_ast_allocator;

	auto top_level_call_ast = parse_expression(expr, ta, ast_allocator);
	auto top_level_call = TypedAST::convert_ast(top_level_call_ast.m_result, typed_ast_allocator);

	// TODO: return a gc_ptr
	eval(top_level_call, env);
	auto value = env.m_env.pop();
	return value_of(value.get());
}

} // namespace Interpreter
