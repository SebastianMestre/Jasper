#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../cst_allocator.hpp"
#include "../lexer.hpp"
#include "../parser.hpp"
#include "../symbol_table.hpp"
#include "../token_array.hpp"
#include "eval.hpp"
#include "execute.hpp"
#include "exit_status_tag.hpp"
#include "interpreter.hpp"
#include "value.hpp"

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

	Interpreter::ExecuteSettings settings;

	ExitStatus exit_code = execute(
	    source,
	    settings,
	    +[](Interpreter::Interpreter& env,
	        Frontend::SymbolTable& context) -> ExitStatus {
		    // NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		    // this means we need to create a call expression node to run the program.
		    // TODO: We need to clean this up

		    {
			    TokenArray const ta = tokenize("__invoke()");

			    CST::Allocator cst_allocator;
			    auto top_level_call_ast = parse_expression(ta, cst_allocator);

			    AST::Allocator ast_allocator;
			    auto top_level_call = AST::convert_ast(top_level_call_ast.m_result, ast_allocator);

				eval(top_level_call, env);
			    auto result = env.m_stack.pop_unsafe();

				Interpreter::print(result);
		    }

		    return ExitStatus::Ok;
	    });

	return static_cast<int>(exit_code);
}
