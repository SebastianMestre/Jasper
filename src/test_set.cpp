#include "test_set.hpp"

TestSet::TestSet(std::string s) : m_source(std::move(s)) {};

TestSet::TestSet(std::string s, TestFunction> tf)
    : m_source(std::move(s)), m_testers({ tf }) {};

TestSet::TestSet(std::string s, std::vector<TestFunction> tfs)
    : m_source(std::move(s)), m_testers(std::move(tfs)) {};



test_type execute(bool dump_ast = false) {
	if (m_testers.empty())
		return test_type::Empty;

	for(auto* f : m_testers)
		if (test_type::Ok != (test_type answer = ::execute(m_source, dump_ast, f)))
			return answer;

	return test_type::Ok;
}
