#include "test_set.hpp"
#include "../interpreter/execute.hpp"

namespace Test {

InterpreterTestSet::InterpreterTestSet(std::string s)
    : m_source(std::move(s)) {};

InterpreterTestSet::InterpreterTestSet(std::string s, Interpret tf)
    : m_source(std::move(s))
    , m_testers({tf}) {};

InterpreterTestSet::InterpreterTestSet(std::string s, std::vector<Interpret> tfs)
    : m_source(std::move(s))
    , m_testers(std::move(tfs)) {};

TestReport InterpreterTestSet::execute() {
	if (m_testers.empty())
		return {TestStatusTag::Empty};

	try {
		for (auto* f : m_testers) {
			ExitStatusTag answer = Interpreter::execute(m_source, m_dump, f);

			if (ExitStatusTag::Ok != answer)
				return { TestStatusTag::Fail,"Error in: " + m_source };
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
