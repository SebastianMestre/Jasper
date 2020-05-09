#include <iostream>
#include <cassert>

#include "tester.hpp"
#include "value.hpp"

namespace Test {

void test_begin() {
	
	Tester bexp_tester(R"(
		int_val := 1 + 2 + 3 + 4;
		float_val := 1.0 + 1.5 + 1.0;
		string_val := "test" + "ing" + ".";
		int_div := 1 / 2;
		float_div := 1.0 / 2.0;
	)");

	bexp_tester.add_test(
		[](Type::Environment env) {
			auto* expected_10 = env.m_scope->access("int_val");
			auto* as_integer = dynamic_cast<Type::Integer*>(expected_10);
			assert(as_integer);
			assert(as_integer->m_value == 10);
			std::cout << "@@ Value is: " << as_integer->m_value << '\n';
			
			auto* expected_3_5 = env.m_scope->access("float_val");
			auto* as_float = dynamic_cast<Type::Float*>(expected_3_5);
			assert(as_float);
			assert(as_float->m_value == 3.5);
			std::cout << "@@ Value is: " << as_float->m_value << '\n';
			
			auto* expected_testing = env.m_scope->access("string_val");
			auto* as_string = dynamic_cast<Type::String*>(expected_testing);
			assert(as_string);
			assert(as_string->m_value == "testing.");
			std::cout << "@@ Value is: " << as_string->m_value << '\n';
			
			auto* expected_0 = env.m_scope->access("int_div");
			as_integer = dynamic_cast<Type::Integer*>(expected_0);
			assert(as_integer);
			assert(as_integer->m_value == 0);
			std::cout << "@@ Value is: " << as_integer->m_value << '\n';
			
			auto* expected_0_5 = env.m_scope->access("float_div");
			as_float = dynamic_cast<Type::Float*>(expected_0_5);
			assert(as_float);
			assert(as_float->m_value == 0.5);
			std::cout << "@@ Value is: " << as_float->m_value << '\n';
			
			return true;	
		}
	);

	assert(bexp_tester.execute(true));
}

}
