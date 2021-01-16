#include <cassert>
#include <iostream>
#include <memory>

#include "../algorithms/tarjan_solver.hpp"
#include "../interpreter/execute.hpp"
#include "../utils/string_set.hpp"
#include "test_status_tag.hpp"
#include "test_utils.hpp"
#include "tester.hpp"

void interpreter_tests(Test::Tester& tests) {
	using TestCase = Test::InterpreterTestSet;
	using Testers = std::vector<Test::Interpret>;

	tests.add_test(std::make_unique<TestCase>("tests/basic_op.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("int_val", env), 10);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("float_val", env), 3.5);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("string_val", env), "testing.");
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("int_div", env), 0);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("float_div", env), 0.5);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_true(eval_expression("litt()", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_false(eval_expression("litf()", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_null(eval_expression("nullv()", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("pizza()", env), 13);
		},
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::is_true(eval_expression("if_else_if()", env));
		},
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("ternary()", env), 2);
	        },
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("array_access()", env), 12);
		},
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("a", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("b", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("c", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("d", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("e", env), -1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("f", env), -1.1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("g", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("h", env), 1.1);
	        }
	    }));

	tests.add_test(std::make_unique<TestCase>("tests/function.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("normal()", env), 3);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("curry()", env), 42);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("I(42)", env), 42);
	        },
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("capture_order()", env), "ABCD");
		},
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("sequence", env), 42);
		}
	    }));

	tests.add_test(std::make_unique<TestCase>("tests/recursion.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("fib(6)", env), 8);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_false(eval_expression("even(11)", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_true(eval_expression("odd(15)", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_true(eval_expression("even(80)", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_false(eval_expression("odd(18)", env));
	        },
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("inner()", env), 2);
		}
	    }));

	tests.add_test(std::make_unique<TestCase>("tests/loops.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("for_loop()", env), 120);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("while_loop()", env), 120);
	        }
	    }));

	tests.add_test(std::make_unique<TestCase>("tests/native.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::array_of_size(eval_expression("append()", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("append()[0]", env), 10);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::array_of_size(eval_expression("extend()", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("extend()[0]", env), 10);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("size_of()", env), 2);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("join()", env), "10,10");
	        }
	    }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>("tests/struct.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("access()", env), "ABCA");
	        }
	    }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>("tests/typesystem.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("a", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::equals(eval_expression("b", env), "hello");
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::array_of_size(eval_expression("c", env), 2);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
	                return Assert::array_of_size(eval_expression("__invoke()", env), 3);
	        },
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("extract()", env), 4);
		}
	    }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>("tests/union.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("serialize(from_number(10))", env), "(number)");
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("serialize(from_string(\"xxx\"))", env), "xxx");
	        },
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("__invoke()", env), 3);
	        },
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("capture_inner()", env), 10);
		}
	    }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>("tests/full_union.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("__invoke()", env), 11);
	        }
	    }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>("tests/simple_language.jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("__invoke()", env), 42);
	        }
	    }));
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
			            TestStatusTag::Fail,
			            "All vertices in a 3-cycle should be in the same SCC"};

		        return {TestStatusTag::Ok};
	        },
	        +[]() -> TestReport {
		        TarjanSolver solver(2);
		        solver.add_edge(0, 1);
		        solver.solve();

		        auto const& cov = solver.component_of_vertices();
		        if (cov[0] == cov[1])
			        return {TestStatusTag::Fail, "Vertices that are only weakly connected should not be in the same SCC"};

		        if (cov[0] < cov[1])
			        return {
			            TestStatusTag::Fail,
			            "SCCs should be in reverse topological sort."};

		        return {TestStatusTag::Ok};
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
			    return {TestStatusTag::Fail, "AAA is not in the set after inserting it"};

		    s.insert("BBB");
		    if (!s.includes("AAA"))
			    return {
			        TestStatusTag::Fail,
			        "AAA is no longer in the set after inserting BBB"};

		    if (!s.includes("BBB"))
			    return {TestStatusTag::Fail, "BBB is not in the set after inserting it"};

		    return {TestStatusTag::Ok};
	    }}));
}

int main() {
	Test::Tester tests;
	tarjan_algorithm_tests(tests);
	allocator_tests(tests);
	string_set_tests(tests);
	interpreter_tests(tests);
	auto test_result = tests.execute();
	if (test_result.m_code != TestStatusTag::Ok)
		return 1;
	return 0;
}
