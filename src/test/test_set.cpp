#include "test_set.hpp"
#include "../interpreter/execute.hpp"

namespace Test {

InterpreterTestSet::InterpreterTestSet(std::string s)
    : m_source(std::move(s)) {};

InterpreterTestSet::InterpreterTestSet(std::string s, Interpret tf)
    : m_source(std::move(s))
    , m_testers({ tf }) {};

InterpreterTestSet::InterpreterTestSet(std::string s, std::vector<Interpret> tfs)
    : m_source(std::move(s))
    , m_testers(std::move(tfs)) {};

TestReport InterpreterTestSet::execute() {
	if (m_testers.empty())
		return { test_status::Empty };

	try {
		for (auto* f : m_testers) {
			exit_status_type answer = Interpreter::execute(m_source, m_dump, f);

			if (exit_status_type::Ok != answer)
				return { test_status::Fail };
		}
	} catch (const std::exception& e) {
		return { test_status::Error, e.what() };
	}

	return { test_status::Ok };
}

NormalTestSet::NormalTestSet() {
}

NormalTestSet::NormalTestSet(std::vector<TestFunction> testers)
    : m_testers { std::move(testers) } {
}

TestReport NormalTestSet::execute() {
	if (m_testers.empty())
		return { test_status::Empty };

	try {
		for (auto* test : m_testers) {
			auto report = test();

			// FUTURE: Accumulate failed tests
			if (report.m_code != test_status::Ok)
				return report;
		}
	} catch (std::exception const& e) {
		return { test_status::Error, e.what() };
	}

	return { test_status::Ok };
}

} // namespace Test
