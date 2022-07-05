#include "execute.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../convert_ast.hpp"
#include "../cst_allocator.hpp"
#include "../frontend_context.hpp"
#include "../lexer.hpp"
#include "../parser.hpp"
#include "../symbol_resolution.hpp"
#include "../symbol_table.hpp"
#include "../token_array.hpp"
#include "../typechecker/ct_eval.hpp"
#include "../typechecker/metacheck.hpp"
#include "../typechecker/typecheck.hpp"
#include "../typechecker/typechecker.hpp"
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
	LexerResult lexer_result = tokenize({source});

	CST::Allocator cst_allocator;
	auto parse_result = parse_program(lexer_result, cst_allocator);

	if (not parse_result.ok()) {
		parse_result.error().print();
		return ExitStatus::ParseError;
	}

	auto cst = parse_result.cst();

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
		auto err = Frontend::resolve_symbols(ast, lexer_result.file_context, context);
		if (!err.ok()) {
			err.print();
			return ExitStatus::StaticError;
		}
	}

	tc.compute_declaration_order(static_cast<AST::Program*>(ast));

	if (settings.typecheck) {
		tc.core().m_meta_core.comp = &tc.declaration_order();
		TypeChecker::metacheck(tc.core().m_meta_core, ast);
		tc.core().m_meta_core.solve();
		TypeChecker::reify_types(ast, tc, ast_allocator);
		TypeChecker::typecheck(ast, tc);
	}

	TypeChecker::compute_offsets(ast, 0);

	GC gc;
	Interpreter env = {&gc, &tc.declaration_order()};
	declare_native_functions(env);
	eval(ast, env);

	return runner(env, context);
}


// FIXME: This might not handle seq-expressions, or inline definitions of
// functions. Investigate.
Value eval_expression(
	const std::string& expr,
	Interpreter& env,
	Frontend::SymbolTable& context
) {
	LexerResult lexer_result = tokenize({expr});

	CST::Allocator cst_allocator;
	auto parse_result = parse_expression(lexer_result, cst_allocator);
	// TODO: handle parse error
	auto cst = parse_result.cst();

	AST::Allocator ast_allocator;
	auto ast = AST::convert_ast(cst, ast_allocator);

	{
		auto err = Frontend::resolve_symbols(ast, lexer_result.file_context, context);
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
