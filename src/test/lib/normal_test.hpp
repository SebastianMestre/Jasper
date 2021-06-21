#pragma once

#include "test_report.hpp"

namespace Test {

struct NormalTest {
	using TestFunction = TestReportPiece (*)();

	NormalTest(TestFunction);
	TestReport execute();

	TestFunction m_tester;
};

} // namespace Test
