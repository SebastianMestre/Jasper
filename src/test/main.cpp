#include "../algorithms/tarjan_solver.hpp"
#include "../utils/string_set.hpp"
#include "lib/normal_test.hpp"
#include "lib/test_set.hpp"
#include "lib/test_status.hpp"

#include "interpreter_tests.hpp"

#include <cassert>
#include <iostream>
#include <memory>

void tarjan_algorithm_tests(Test::TestSet& tester) {
	Test::TestSet inner_tester;

	auto add_test = [&](Test::NormalTest::TestFunction function) {
		inner_tester.add_test(Test::NormalTest(function));
	};

	add_test(+[]() -> TestReportPiece {
		TarjanSolver solver(3);
		solver.add_edge(0, 1);
		solver.add_edge(1, 2);
		solver.add_edge(2, 0);
		solver.solve();

		auto const& cov = solver.component_of_vertices();

		if (cov[0] != cov[1] || cov[0] != cov[2])
			return {
			    TestStatus::Fail,
			    "All vertices in a 3-cycle should be in the same SCC"};

		return {TestStatus::Ok};
	});

	add_test(+[]() -> TestReportPiece {
		TarjanSolver solver(2);
		solver.add_edge(0, 1);
		solver.solve();

		auto const& cov = solver.component_of_vertices();
		if (cov[0] == cov[1])
			return {TestStatus::Fail, "Vertices that are only weakly connected should not be in the same SCC"};

		if (cov[0] < cov[1])
			return {TestStatus::Fail, "SCCs should be in reverse topological sort."};

		return {TestStatus::Ok};
	});

	tester.add_test(std::move(inner_tester));
}

void allocator_tests(Test::TestSet& tests) {
	// TODO
}

void string_set_tests(Test::TestSet& tester) {
	Test::TestSet inner_tester;

	auto add_test = [&](Test::NormalTest::TestFunction function) {
		inner_tester.add_test(Test::NormalTest(function));
	};

	add_test(+[]() -> TestReportPiece {
		StringSet s;
		s.insert("AAA");
		if (!s.includes("AAA"))
			return {TestStatus::Fail, "AAA is not in the set after inserting it"};

		s.insert("BBB");
		if (!s.includes("AAA"))
			return {TestStatus::Fail, "AAA is no longer in the set after inserting BBB"};

		if (!s.includes("BBB"))
			return {TestStatus::Fail, "BBB is not in the set after inserting it"};

		return {TestStatus::Ok};
	});

	tester.add_test(std::move(inner_tester));
}

int main() {
	Test::TestSet tests;

	tarjan_algorithm_tests(tests);
	allocator_tests(tests);
	string_set_tests(tests);
	interpreter_tests(tests);

	auto test_report = tests.execute();
	test_report.print();
	auto test_status = test_report.compute_status();
	if (test_status != TestStatus::Ok)
		return 1;
	return 0;
}
