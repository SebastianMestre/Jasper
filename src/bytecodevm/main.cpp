#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../ast_allocator.hpp"
#include "../compute_offsets.hpp"
#include "../ct_eval.hpp"
#include "../desugar.hpp"
#include "../interpreter/exit_status_tag.hpp"
#include "../interpreter/garbage_collector.hpp"
#include "../interpreter/gc_ptr.hpp"
#include "../interpreter/interpreter.hpp"
#include "../interpreter/native.hpp"
#include "../interpreter/value.hpp"
#include "../match_identifiers.hpp"
#include "../metacheck.hpp"
#include "../parse.hpp"
#include "../parser.hpp"
#include "../token_array.hpp"
#include "../typecheck.hpp"
#include "../typechecker.hpp"
#include "../typed_ast.hpp"
#include "../typed_ast_allocator.hpp"
#include "bytecode.hpp"
#include "compile.hpp"
#include "run_bytecode.hpp"

int main(int argc, char** argv) {

	if (argc < 2) {
		std::cout << "Argument missing: source file" << std::endl;
		return 1;
	}

	std::ifstream in_fs(argv[1]);
	if (!in_fs.good()) {
		std::cout << "Failed to open '" << argv[1] << "'" << std::endl;
		return 1;
	}

	std::stringstream file_content;
	std::string line;

	while (std::getline(in_fs, line)) {
		file_content << line << '\n';
	}

	std::string source = file_content.str();

	TokenArray ta;
	AST::Allocator ast_allocator;
	auto parse_result = parse_program(source, ta, ast_allocator);

	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return int(ExitStatusTag::ParseError);
	}

	// Can this even happen? parse_program should always either return a
	// DeclarationList or an error
	if (parse_result.m_result->type() != ASTTag::DeclarationList)
		return int(ExitStatusTag::TopLevelTypeError);

	auto desugared_ast = AST::desugar(parse_result.m_result, ast_allocator);

	TypedAST::Allocator typed_ast_allocator;
	auto top_level = TypedAST::convert_ast(desugared_ast, typed_ast_allocator);
	TypeChecker::TypeChecker tc{typed_ast_allocator};

	{
		auto err = TypeChecker::match_identifiers(top_level, tc.m_env);
		if (!err.ok()) {
			err.print();
			return int(ExitStatusTag::StaticError);
		}
	}

	tc.m_env.compute_declaration_order(static_cast<TypedAST::DeclarationList*>(top_level));

	TypeChecker::metacheck(top_level, tc);
	top_level = TypeChecker::ct_eval(top_level, tc, typed_ast_allocator);
	TypeChecker::typecheck(top_level, tc);
	TypeChecker::compute_offsets(top_level, 0);

	auto bytecode = emit_bytecode(top_level);

	Interpreter::GC gc;
	Interpreter::Interpreter env {&tc, &gc};
	Interpreter::declare_native_functions(env);
	BytecodeRunner bc_runner {env};
	int runner_exit_code = bc_runner.run(bytecode);
	if (runner_exit_code)
		return runner_exit_code;

	// runner_exit_code =
	    // bc_runner.run({{Opcode::GlobalAccess, 0, "__invoke"}, {Opcode::Call, 0}});

	
	auto invoke = env.global_access("__invoke");
	Interpreter::print(invoke);

	return runner_exit_code;
}
