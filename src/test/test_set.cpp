#include "test_set.hpp"
#include "../interpreter/execute.hpp"

namespace Test {

TestSet::TestSet(std::string s) : m_source(std::move(s)) {};

TestSet::TestSet(std::string s, TestFunction tf)
    : m_source(std::move(s)), m_testers({ tf }) {};

TestSet::TestSet(std::string s, std::vector<TestFunction> tfs)
    : m_source(std::move(s)), m_testers(std::move(tfs)) {};



exit_status_type TestSet::execute(bool dump_ast) {
	if (m_testers.empty())
		return exit_status_type::Empty;

	for(auto* f : m_testers) {
		exit_status_type answer = Interpreter::execute(m_source, dump_ast, f);

		if (exit_status_type::Ok != answer)
			return answer;
	}

	return exit_status_type::Ok;
}

}
