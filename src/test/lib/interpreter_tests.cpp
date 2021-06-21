#include "interpreter_tests.hpp"

#include <fstream>
#include <sstream>

#include "../../interpreter/execute.hpp"
#include "../../symbol_table.hpp"

namespace Test {

InterpreterTestSet::InterpreterTestSet(std::string s)
    : m_source_file(std::move(s)) {};

InterpreterTestSet::InterpreterTestSet(std::string s, Interpret tf)
    : m_source_file(std::move(s))
    , m_testers({tf}) {};

InterpreterTestSet::InterpreterTestSet(std::string s, std::vector<Interpret> tfs)
    : m_source_file(std::move(s))
    , m_testers(std::move(tfs)) {};

TestReport InterpreterTestSet::execute() {
	if (m_testers.empty())
		return {TestStatus::Empty};

	try {
		std::ifstream in_fs(m_source_file);
		if (!in_fs.good())
			return {TestStatus::MissingFile};

		std::stringstream file_content;
		std::string line;

		while (std::getline(in_fs, line))
			file_content << line << '\n';

		Interpreter::ExecuteSettings settings;
		settings.dump_cst = m_dump;

		for (auto* f : m_testers) {
			ExitStatus answer = Interpreter::execute(file_content.str(), settings, f);

			if (ExitStatus::Ok != answer)
				return {TestStatus::Fail};
		}
	} catch (const std::exception& e) {
		return {TestStatus::Error, e.what()};
	}

	return {TestStatus::Ok};
}

} // namespace Test
