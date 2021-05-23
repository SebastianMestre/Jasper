#include <fstream>
#include <sstream>

#include "../interpreter/execute.hpp"
#include "../symbol_table.hpp"
#include "test_set.hpp"

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
		return {TestStatusTag::Empty};

	try {
		std::ifstream in_fs(m_source_file);
		if (!in_fs.good())
			return {TestStatusTag::MissingFile};

		std::stringstream file_content;
		std::string line;

		while (std::getline(in_fs, line))
			file_content << line << '\n';

		Interpreter::ExecuteSettings settings;
		settings.dump_cst = m_dump;

		for (auto* f : m_testers) {
			ExitStatusTag answer = Interpreter::execute(file_content.str(), settings, f);

			if (ExitStatusTag::Ok != answer)
				return {TestStatusTag::Fail};
		}
	} catch (const std::exception& e) {
		return {TestStatusTag::Error, e.what()};
	}

	return {TestStatusTag::Ok};
}

NormalTestSet::NormalTestSet() {}

NormalTestSet::NormalTestSet(TestFunction tester)
    : m_testers {tester} {}

NormalTestSet::NormalTestSet(std::vector<TestFunction> testers)
    : m_testers {std::move(testers)} {}

TestReport NormalTestSet::execute() {
	if (m_testers.empty())
		return {TestStatusTag::Empty};

	try {
		for (auto* test : m_testers) {
			auto report = test();

			// FUTURE: Accumulate failed tests
			if (report.m_code != TestStatusTag::Ok)
				return report;
		}
	} catch (std::exception const& e) {
		return {TestStatusTag::Error, e.what()};
	}

	return {TestStatusTag::Ok};
}

} // namespace Test
