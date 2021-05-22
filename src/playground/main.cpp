#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../cst_allocator.hpp"
#include "../lexer.hpp"
#include "../parser.hpp"
#include "../token_array.hpp"

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

	{
		TokenArray const ta = tokenize(source.c_str());

		CST::Allocator cst_allocator;
		auto parse_result = parse_program(ta, cst_allocator);

		if (not parse_result.ok()) {
			parse_result.m_error.print();
			return 1;
		}

		print(parse_result.m_result, 0);
		std::cout << "\n";
	}

	return 0;
}
