#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "environment.hpp"
#include "eval.hpp"
#include "execute.hpp"
#include "garbage_collector.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

int main() {

	std::ifstream in_fs("source.jp");

	std::stringstream file_content;
	std::string line;

	while(std::getline(in_fs, line)){
		file_content << line << '\n';
	}

	std::string source = file_content.str();


	int exit_code = execute(source, true, +[](Type::Environment& env) -> int {

		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up

		{
			auto top_level_name = std::make_unique<ASTIdentifier>();
			top_level_name->m_text = "__invoke";

			auto top_level_call = std::make_unique<ASTCallExpression>();
			top_level_call->m_callee = std::move(top_level_name);
			top_level_call->m_args = std::make_unique<ASTArgumentList>();

			eval(top_level_call.get(), env);
		}

		return 0;
	});

	return exit_code;
}
