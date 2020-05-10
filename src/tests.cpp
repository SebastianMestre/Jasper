#include <iostream>
#include <cassert>

#include "eval.hpp"
#include "tester.hpp"
#include "value.hpp"

int main() {
	using namespace Test;
	
	Tester bexp_tester(R"(
		int_val := 1 + 2 + 3 + 4;
		float_val := 1.0 + 1.5 + 1.0;
		string_val := "test" + "ing" + ".";
		int_div := 1 / 2;
		float_div := 1.0 / 2.0;
	)");

	bexp_tester.add_test(
		+[](Type::Environment& env) -> bool {
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

	Tester monolithic_test = {R"(
	x : dec = 1.4;
	y : int = 3;
	z := fn () {
		a := 2;
		b : dec = (4 + 5) * 3 * 1 + 4.5 * 2;
		c := a;
		a + b;
		cmp : bool = a < c;

		sqrt5 := obt{x:=1;y:=2;} |> norm();
		sixteen := my_lib.times_pi(my_lib.five()) + 1;
	};

	y := fn () {
		print(1,z(),4);
	};

	w := fn (a,b:int,c){};

	f := obt {
		greeting := "Hello, ";
		__invoke := fn (name : string) {
			print(greeting + name);
		};
	};

	f := fn (name : string) {};

	sqrt := fn (x) { return (x+1) * 0.5; };

	norm := fn(p){
		return sqrt(p.x * p.x + p.y * p.y);
	};

	my_lib := dict {
		five := fn() { return 6; };
		times_pi := fn(x) { return x * 3; };
	};

	__invoke := fn () {
		f.greeting = "Hey, ";
		f("Sailor");
	};

	names := dict {
		user0 := "peter";
		user1 := "joseph";
		user2 := "anne";
	};

)"};

	monolithic_test.add_test(+[](Type::Environment& env) -> bool {
		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up
		auto top_level_name = std::make_unique<ASTIdentifier>();
		top_level_name->m_text = "__invoke";

		auto top_level_call = std::make_unique<ASTCallExpression>();
		top_level_call->m_callee = std::move(top_level_name);
		top_level_call->m_args = std::make_unique<ASTArgumentList>();

		eval(top_level_call.get(), env);

		return true;
	});

	Tester function_return{R"(
	f := fn() {
		a := 1;
		b := 2;
		return a + b;
	}
)"};
	
	function_return.add_test(+[](Type::Environment& env) -> bool {
		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up
		auto top_level_name = std::make_unique<ASTIdentifier>();
		top_level_name->m_text = "f";

		auto top_level_call = std::make_unique<ASTCallExpression>();
		top_level_call->m_callee = std::move(top_level_name);
		top_level_call->m_args = std::make_unique<ASTArgumentList>();

		Type::Integer* returned = eval(top_level_call.get(), env);

		return returned->m_value == 3;
	});

	assert(bexp_tester.execute(true));
	assert(monolithic_test.execute(true));
	assert(function_return.execute(true));
}
