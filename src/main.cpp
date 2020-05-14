#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "environment.hpp"
#include "eval.hpp"
#include "execute.hpp"
#include "parse.hpp"
#include "token_array.hpp"
#include "value.hpp"

int main() {

	std::ifstream in_fs("source.jp");

	std::stringstream file_content;
	std::string line;

	while(std::getline(in_fs, line)){
		file_content << line << '\n';
	}

	std::string source = file_content.str();

	int exit_code = execute(source, false, +[](Type::Environment& env) -> int {

		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up

		{
			TokenArray ta;
			auto top_level_call = parse_expression("__invoke()", ta);

			auto* result = eval(top_level_call.m_result.get(), env);

			if (result)
				Type::print(result);
			else
				std::cout << "(nullptr)\n";
		}

		return 0;
	});

	return exit_code;
}
