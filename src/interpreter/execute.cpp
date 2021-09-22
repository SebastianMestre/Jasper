#include "execute.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../cst_allocator.hpp"
#include "../ct_eval.hpp"
#include "../lexer.hpp"
#include "../match_identifiers.hpp"
#include "../metacheck.hpp"
#include "../parser.hpp"
#include "../symbol_table.hpp"
#include "../token_array.hpp"
#include "../typecheck.hpp"
#include "../typechecker.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "native.hpp"
#include "utils.hpp"

namespace Interpreter {

ExitStatus execute(
	std::string const& source,
	ExecuteSettings settings,
	Runner* runner
) {
	TokenArray const ta = tokenize(source.c_str());

	CST::Allocator cst_allocator;
	auto parse_result = parse_program(ta, cst_allocator);

	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return ExitStatus::ParseError;
	}

	auto cst = parse_result.m_result;

	if (settings.dump_cst)
		print(cst, 1);

	// Can this even happen? parse_program should always either return a
	// Program or an error
	if (cst->type() != CSTTag::Program)
		return ExitStatus::TopLevelTypeError;

	AST::Allocator ast_allocator;
	auto ast = AST::convert_ast(cst, ast_allocator);

	// creates and stores a bunch of builtin declarations
	TypeChecker::TypeChecker tc{ast_allocator};
	Frontend::SymbolTable context;

	{
		for (auto& bucket : tc.m_builtin_declarations.m_buckets)
			for (auto& decl : bucket)
				context.declare(&decl);
		auto err = Frontend::match_identifiers(ast, context);
		if (!err.ok()) {
			err.print();
			return ExitStatus::StaticError;
		}
	}

	tc.m_env.compute_declaration_order(static_cast<AST::Program*>(ast));

	if (settings.typecheck) {
		tc.m_core.m_meta_core.comp = &tc.m_env.declaration_components;
		TypeChecker::metacheck(tc.m_core.m_meta_core, ast);
		TypeChecker::reify_types(ast, tc, ast_allocator);
		TypeChecker::typecheck(ast, tc);
	}

	TypeChecker::compute_offsets(ast, 0);

	GC gc;
	Interpreter env = {&tc, &gc, &tc.m_env.declaration_components};
	declare_native_functions(env);
	eval(ast, env);

	return runner(env, context);
}


// FIXME: This does not handle seq-expressions, or inline definitions of
// functions, because it does not call `match_identifiers` or `compute_offsets`.
// Note that we can't just call match_identifiers, because that wouldn't take
// into account the rest of the program that's already been processed, before
// this is run
Value eval_expression(
	const std::string& expr,
	Interpreter& env,
	Frontend::SymbolTable& context
) {
	TokenArray const ta = tokenize(expr.c_str());

	CST::Allocator cst_allocator;
	auto parse_result = parse_expression(ta, cst_allocator);
	// TODO: handle parse error
	auto cst = parse_result.m_result;

	AST::Allocator ast_allocator;
	auto ast = AST::convert_ast(cst, ast_allocator);

	{
		auto err = Frontend::match_identifiers(ast, context);
		if (!err.ok()) {
			err.print();
			return env.null();
		}
	}

	TypeChecker::compute_offsets(ast, 0);

	// TODO?: return a gc_ptr
	eval(ast, env);
	auto value = env.m_stack.pop_unsafe();
	return value_of(value);
}

} // namespace Interpreter
