#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../parse.hpp"
#include "../token_array.hpp"
#include "../typed_ast.hpp"
#include "environment.hpp"
#include "eval.hpp"
#include "execute.hpp"
#include "exit_status_tag.hpp"
#include "value.hpp"

int main() {

	std::ifstream in_fs("source.jp");

	std::stringstream file_content;
	std::string line;

	while (std::getline(in_fs, line)) {
		file_content << line << '\n';
	}

	std::string source = file_content.str();

	ExitStatusTag exit_code = execute(
	    source, false, +[](Interpreter::Environment& env) -> ExitStatusTag {
		    // NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		    // this means we need to create a call expression node to run the program.
		    // TODO: We need to clean this up

		    {
			    TokenArray ta;
			    auto top_level_call_ast = parse_expression("__invoke()", ta);
			    auto top_level_call =
			        TypedAST::convertAST(top_level_call_ast.m_result.get());

			    auto result = eval(top_level_call.get(), env);

			    if (result)
				    Interpreter::print(result.get());
			    else
				    std::cout << "(nullptr)\n";
		    }

		    return ExitStatusTag::Ok;
	    });

	return static_cast<int>(exit_code);
}
