#include "interpreter_tests.hpp"

#include "../interpreter/execute.hpp"
#include "lib/interpreter_tests.hpp"
#include "lib/test_set.hpp"
#include "lib/test_utils.hpp"

#include <vector>

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

void interpreter_tests(Test::TestSet& tests) {

	auto add_test = [&](
		char const* file,
		std::vector<Test::InterpreterTestSet::Interpret> testers
	) {
		tests.add_test(Test::InterpreterTestSet(file, std::move(testers)));
	};

	add_test(
	    "tests/basic_op.jp",
	    {
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
	    });

	add_test(
	    "tests/function.jp",
	    {
	        EQUALS("normal()", 3),
	        EQUALS("curry()", 42),
	        EQUALS("I(42)", 42),
	        EQUALS("capture_order()", "ABCD"),
	        EQUALS("median_of_three(1,2,3)", 2),
	        EQUALS("median_of_three(3,2,1)", 2),
	        EQUALS("median_of_three(10,15,7)", 10),
	        EQUALS("second(15,7)", 7),
	        EQUALS("second(7,15)", 15),
	    });

	add_test(
	    "tests/recursion.jp",
	    {
	        EQUALS("fib(6)", 8),
	        IS_FALSE("even(11)"),
	        IS_TRUE("odd(15)"),
	        IS_TRUE("even(80)"),
	        IS_FALSE("odd(18)"),
	        EQUALS("inner()", 2),
	    });

	add_test(
	    "tests/loops.jp",
	    {
	        EQUALS("for_loop()", 120),
	        EQUALS("while_loop()", 120),
	    });

	add_test(
	    "tests/native.jp",
	    {
	        ARRAY_OF_SIZE("append()", 1),
	        EQUALS("append()[0]", 10),
	        ARRAY_OF_SIZE("extend()", 1),
	        EQUALS("extend()[0]", 10),
	        EQUALS("size_of()", 2),
	        EQUALS("join()", "10,10"),
	    });

	add_test("tests/struct.jp", {EQUALS("access()", "ABCA")});

	add_test(
	    "tests/typesystem.jp",
	    {
	        EQUALS("a", 1),
	        EQUALS("b", "hello"),
	        ARRAY_OF_SIZE("c", 2),
	        ARRAY_OF_SIZE("__invoke()", 3),
	        EQUALS("extract()", 4),
	    });

	add_test(
	    "tests/union.jp",
	    {
	        EQUALS("serialize(from_number(10))", "(number)"),
	        EQUALS("serialize(from_string(\"xxx\"))", "xxx"),
	        EQUALS("__invoke()", 3),
	        EQUALS("capture_inner()", 10),
	    });

	add_test("tests/full_union.jp", {EQUALS("__invoke()", 11)});

	add_test("tests/simple_language.jp", {EQUALS("__invoke()", 42)});

	add_test(
	    "tests/seq_expressions.jp",
	    {
	        EQUALS("return_const", 31415),
	        EQUALS("return_call", 42),
	        EQUALS("issue232_1()", 6),
	        EQUALS("issue232_2()", 7),
	        EQUALS("issue240_1", 10),
	        EQUALS("issue240_2", 8),
	    });
}
