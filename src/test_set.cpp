#include "test_set.hpp"
#include "execute.hpp"

namespace Test {

TestSet::TestSet(std::string s) : m_source(std::move(s)) {};

TestSet::TestSet(std::string s, TestFunction tf)
    : m_source(std::move(s)), m_testers({ tf }) {};

TestSet::TestSet(std::string s, std::vector<TestFunction> tfs)
    : m_source(std::move(s)), m_testers(std::move(tfs)) {};



test_type TestSet::execute(bool dump_ast) {
	if (m_testers.empty())
		return test_type::Empty;

	for(auto* f : m_testers) {
		test_type answer = ::execute(m_source, dump_ast, f);

		if (test_type::Ok != answer)
			return answer;
	}

	return test_type::Ok;
}

}
