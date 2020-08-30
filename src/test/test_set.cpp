#include "test_set.hpp"
#include "../interpreter/execute.hpp"

namespace Test {

TestSet::TestSet(Run tf)
    : m_runners({ tf }) {};

TestSet::TestSet(std::vector<Run> tfs)
    : m_runners(std::move(tfs)) {};



TestReport TestSet::execute() {
	if (m_runners.empty())
		return { test_status::Empty };

	try {
		for(auto* f : m_runners) {
			TestReport answer = f();

			if (test_status::Ok != answer.m_code)
				return answer;
		}
	} catch(const std::exception& e) {
		return { test_status::Error, e.what() };
	}

	return { test_status::Ok };
}



TestInterpreter::TestInterpreter(std::string s) : m_source(std::move(s)) {};

TestInterpreter::TestInterpreter(std::string s, Interpret tf)
    : m_source(std::move(s)), m_testers({ tf }) {};

TestInterpreter::TestInterpreter(std::string s, std::vector<Interpret> tfs)
    : m_source(std::move(s)), m_testers(std::move(tfs)) {};



TestReport TestInterpreter::execute() {
	if (m_testers.empty())
		return { test_status::Empty };

	try {
		for(auto* f : m_testers) {
			exit_status_type answer = Interpreter::execute(m_source, m_dump, f);

			if (exit_status_type::Ok != answer)
				return { test_status::Fail };
		}
	} catch(const std::exception& e) {
		return { test_status::Error, e.what() };
	}

	return { test_status::Ok };
}

}
