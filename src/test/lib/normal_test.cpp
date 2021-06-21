#include "normal_test.hpp"

#include <cassert>

namespace Test {

NormalTest::NormalTest(TestFunction tester)
    : m_tester {tester} {
	assert(tester);
}

TestReport NormalTest::execute() {
	try {
		return m_tester();
	} catch (std::exception const& e) {
		return {TestStatus::Error, e.what()};
	}
}

} // namespace Test
