#include <cassert>
#include <iostream>
#include <memory>

#include "../algorithms/tarjan_solver.hpp"
#include "../interpreter/execute.hpp"
#include "../utils/string_set.hpp"
#include "test_status_tag.hpp"
#include "test_utils.hpp"
#include "tester.hpp"

#define EQUALS(expr, value)                                                    \
	+[](Interpreter::Interpreter& env,                                         \
	    Frontend::SymbolTable& context) -> ExitStatus {                        \
		return Assert::equals(eval_expression(expr, env, context), value);     \
	}

#define IS_TRUE(expr)                                                          \
	+[](Interpreter::Interpreter& env,                                         \
	    Frontend::SymbolTable& context) -> ExitStatus {                        \
		return Assert::is_true(eval_expression(expr, env, context));           \
	}

#define IS_FALSE(expr)                                                         \
	+[](Interpreter::Interpreter& env,                                         \
	    Frontend::SymbolTable& context) -> ExitStatus {                        \
		return Assert::is_false(eval_expression(expr, env, context));          \
	}

#define IS_NULL(expr)                                                          \
	+[](Interpreter::Interpreter& env,                                         \
	    Frontend::SymbolTable& context) -> ExitStatus {                        \
		return Assert::is_null(eval_expression(expr, env, context));           \
	}

#define ARRAY_OF_SIZE(expr, size)                                                \
	+[](Interpreter::Interpreter& env,                                           \
	    Frontend::SymbolTable& context) -> ExitStatus {                          \
		return Assert::array_of_size(eval_expression(expr, env, context), size); \
	}

void interpreter_tests(Test::Tester& tests) {
	    using TestCase = Test::InterpreterTestSet;
	    using Testers = std::vector<Test::Interpret>;

	    tests.add_test(std::make_unique<TestCase>(
	        "tests/basic_op.jp",
	        Testers {
	            EQUALS("int_val", 10),
	            EQUALS("float_val", 3.5),
	            EQUALS("string_val", "testing."),
	            EQUALS("int_div", 0),
	            EQUALS("float_div", 0.5),
	            IS_TRUE("logic_ops()"),
	            IS_TRUE("litt()"),
	            IS_FALSE("litf()"),
	            IS_NULL("nullv()"),
	            EQUALS("pizza()", 13),
	            EQUALS("issue261()", 13),
	            IS_TRUE("if_else_if()"),
	            EQUALS("ternary()", 2), //
	            EQUALS("array_access()", 12),
	            EQUALS("a", 1),
	            EQUALS("b", 1),
	            EQUALS("c", 1),
	            EQUALS("d", 1),
	            EQUALS("e", -1),
	            EQUALS("f", -1.1),
	            EQUALS("g", 1),
	            EQUALS("h", 1.1),
	            EQUALS("ternary_disambiguations", 1),
	        }));

	    tests.add_test(std::make_unique<TestCase>(
	        "tests/function.jp",
	        Testers {
	            EQUALS("normal()", 3),
	            EQUALS("curry()", 42),
	            EQUALS("I(42)", 42),
	            EQUALS("capture_order()", "ABCD"),
	            EQUALS("median_of_three(1,2,3)", 2),
	            EQUALS("median_of_three(3,2,1)", 2),
	            EQUALS("median_of_three(10,15,7)", 10),
	            EQUALS("second(15,7)", 7),
	            EQUALS("second(7,15)", 15),
	        }));

	    tests.add_test(std::make_unique<TestCase>(
	        "tests/recursion.jp",
	        Testers {
	            EQUALS("fib(6)", 8),
	            IS_FALSE("even(11)"),
	            IS_TRUE("odd(15)"),
	            IS_TRUE("even(80)"),
	            IS_FALSE("odd(18)"),
	            EQUALS("inner()", 2)}));

	    tests.add_test(std::make_unique<TestCase>(
	        "tests/loops.jp",
	        Testers {EQUALS("for_loop()", 120), EQUALS("while_loop()", 120)}));

	    tests.add_test(std::make_unique<TestCase>(
	        "tests/native.jp",
	        Testers {
	            ARRAY_OF_SIZE("append()", 1),
	            EQUALS("append()[0]", 10),
	            ARRAY_OF_SIZE("extend()", 1),
	            EQUALS("extend()[0]", 10),
	            EQUALS("size_of()", 2),
	            EQUALS("join()", "10,10")}));

	    tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	        "tests/struct.jp", Testers {EQUALS("access()", "ABCA")}));

	    tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	        "tests/typesystem.jp",
	        Testers {
	            EQUALS("a", 1),
	            EQUALS("b", "hello"),
	            ARRAY_OF_SIZE("c", 2),
	            ARRAY_OF_SIZE("__invoke()", 3),
	            EQUALS("extract()", 4)}));

	    tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	        "tests/union.jp",
	        Testers {
	            EQUALS("serialize(from_number(10))", "(number)"),
	            EQUALS("serialize(from_string(\"xxx\"))", "xxx"),
	            EQUALS("__invoke()", 3),
	            EQUALS("capture_inner()", 10)}));

	    tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	        "tests/full_union.jp", Testers {EQUALS("__invoke()", 11)}));

	    tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	        "tests/simple_language.jp", Testers {EQUALS("__invoke()", 42)}));

	    tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	        "tests/seq_expressions.jp",
	        Testers {
	            EQUALS("return_const", 31415),
	            EQUALS("return_call", 42),
	            EQUALS("issue232_1()", 6),
	            EQUALS("issue232_2()", 7),
	            EQUALS("issue240_1", 10),
	            EQUALS("issue240_2", 8)}));
}

void tarjan_algorithm_tests(Test::Tester& tester) {
	tester.add_test(std::make_unique<Test::NormalTestSet>(
	    std::vector<Test::NormalTestSet::TestFunction> {
	        +[]() -> TestReport {
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
	        },
	        +[]() -> TestReport {
		        TarjanSolver solver(2);
		        solver.add_edge(0, 1);
		        solver.solve();

		        auto const& cov = solver.component_of_vertices();
		        if (cov[0] == cov[1])
			        return {TestStatus::Fail, "Vertices that are only weakly connected should not be in the same SCC"};

		        if (cov[0] < cov[1])
			        return {
			            TestStatus::Fail,
			            "SCCs should be in reverse topological sort."};

		        return {TestStatus::Ok};
	        }}));
}

void allocator_tests(Test::Tester& tests) {
	// TODO
}

void string_set_tests(Test::Tester& tester) {
	tester.add_test(std::make_unique<Test::NormalTestSet>(
	    std::vector<Test::NormalTestSet::TestFunction> {+[]() -> TestReport {
		    StringSet s;
		    s.insert("AAA");
		    if (!s.includes("AAA"))
			    return {TestStatus::Fail, "AAA is not in the set after inserting it"};

		    s.insert("BBB");
		    if (!s.includes("AAA"))
			    return {
			        TestStatus::Fail,
			        "AAA is no longer in the set after inserting BBB"};

		    if (!s.includes("BBB"))
			    return {TestStatus::Fail, "BBB is not in the set after inserting it"};

		    return {TestStatus::Ok};
	    }}));
}

int main() {
	Test::Tester tests;
	tarjan_algorithm_tests(tests);
	allocator_tests(tests);
	string_set_tests(tests);
	interpreter_tests(tests);
	auto test_result = tests.execute();
	if (test_result.m_code != TestStatus::Ok)
		return 1;
	return 0;
}

#undef EQUALS
#undef IS_TRUE
#undef IS_FALSE
#undef IS_NULL
#undef ARRAY_OF_SIZE
