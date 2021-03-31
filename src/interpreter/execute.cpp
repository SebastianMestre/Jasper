#include "execute.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../cst_allocator.hpp"
#include "../ct_eval.hpp"
#include "../match_identifiers.hpp"
#include "../metacheck.hpp"
#include "../parser.hpp"
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

	TokenArray ta = tokenize(source);

	CST::Allocator cst_allocator;
	auto parse_result = parse_program(ta, cst_allocator);

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

	// creates and stores a bunch of builtin declarations
	TypeChecker::TypeChecker tc{ast_allocator};

	{
		auto err = Frontend::match_identifiers(ast, tc.m_builtin_declarations);
		if (!err.ok()) {
			err.print();
			return ExitStatusTag::StaticError;
		}
	}

	tc.m_env.compute_declaration_order(static_cast<AST::DeclarationList*>(ast));

	// TODO: expose this flag
	constexpr bool run_typechecking = true;
	if (run_typechecking) {
		TypeChecker::metacheck(ast, tc);
		ast = TypeChecker::ct_eval(ast, tc, ast_allocator);
		TypeChecker::typecheck(ast, tc);
	}
	TypeChecker::compute_offsets(ast, 0);

	GC gc;
	Interpreter env = {&tc, &gc, &tc.m_env.declaration_components};
	declare_native_functions(env);
	eval(ast, env);

	ExitStatusTag runner_exit_code = runner(env);

	return runner_exit_code;
}

// FIXME: This does not handle seq-expressions, or inline definitions of
// functions, because it does not call `match_identifiers` or `compute_offsets`.
// Note that we can't just call match_identifiers, because that wouldn't take
// into account the rest of the program that's already been processed, before
// this is run
Value* eval_expression(const std::string& expr, Interpreter& env) {
	TokenArray ta = tokenize(expr);

	CST::Allocator cst_allocator;
	auto parse_result = parse_expression(ta, cst_allocator);
	// TODO: handle parse error
	auto cst = parse_result.m_result;

	AST::Allocator ast_allocator;
	auto ast = AST::convert_ast(cst, ast_allocator);

	// TODO?: return a gc_ptr
	eval(ast, env);
	auto value = env.m_stack.pop();
	return value_of(value.get());
}

} // namespace Interpreter
