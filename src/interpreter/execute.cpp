#include "execute.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../convert_ast.hpp"
#include "../cst_allocator.hpp"
#include "../frontend_context.hpp"
#include "../lexer.hpp"
#include "../log/log.hpp"
#include "../parser.hpp"
#include "../symbol_resolution.hpp"
#include "../symbol_table.hpp"
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
	CST::Allocator cst_allocator;
	AST::Allocator ast_allocator;

	LexerResult lexer_result = tokenize({source});

	auto parse_result = parse_program(std::move(lexer_result), cst_allocator);

	if (not parse_result.ok()) {
		parse_result.error().print();
		return ExitStatus::ParseError;
	}

	if (settings.dump_cst)
		print(parse_result.cst(), 1);

	auto ast = AST::convert_program(parse_result.cst(), ast_allocator);

	// creates and stores a bunch of builtin declarations
	TypeChecker::TypeChecker tc{ast_allocator};
	Frontend::SymbolTable context;

	{
		for (auto& bucket : tc.m_builtin_declarations.m_buckets)
			for (auto& decl : bucket)
				context.declare(&decl);
		auto err = Frontend::resolve_symbols_program(ast, parse_result.file_context(), context);
		if (!err.ok()) {
			err.print();
			return ExitStatus::StaticError;
		}
	}

	tc.compute_declaration_order(ast);

	if (settings.typecheck) {
		TypeChecker::metacheck_program(ast);
		TypeChecker::reify_types(ast, tc);
		TypeChecker::typecheck_program(ast, tc);
	}

	TypeChecker::compute_offsets_program(ast, 0);

	GC gc;
	Interpreter env = {&gc, &tc.declaration_order()};
	declare_native_functions(env);
	run(ast, env);

	return runner(env, context);
}


// FIXME: This might not handle seq-expressions, or inline definitions of
// functions. Investigate.
Value eval_expression(
	const std::string& expr,
	Interpreter& env,
	Frontend::SymbolTable& context
) {
	CST::Allocator cst_allocator;
	AST::Allocator ast_allocator;

	LexerResult lexer_result = tokenize({expr});

	auto parse_result = parse_expression(std::move(lexer_result), cst_allocator);

	if (!parse_result.ok()) {
		parse_result.error().print();
		Log::fatal() << "parser error";
	}

	auto ast = AST::convert_expr(parse_result.cst(), ast_allocator);

	{
		auto err = Frontend::resolve_symbols(ast, parse_result.file_context(), context);
		if (!err.ok()) {
			err.print();
			return env.null();
		}
	}

	TypeChecker::compute_offsets(ast, 0);

	// TODO?: return a gc_ptr
	eval(ast, env);
	auto value = env.m_stack.pop_unsafe();
	return value;
}

} // namespace Interpreter
