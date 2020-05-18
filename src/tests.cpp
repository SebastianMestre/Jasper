#include <iostream>
#include <cassert>

#include "eval.hpp"
#include "parse.hpp"
#include "tester.hpp"
#include "token_array.hpp"
#include "value.hpp"

int main() {
	using namespace Test;

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

	monolithic_test.add_test(+[](Type::Environment& env) -> int {
		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up
		TokenArray ta;
		auto top_level_call = parse_expression("__invoke()", ta);

		eval(top_level_call.m_result.get(), env);

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == monolithic_test.execute(false));

	Tester bexp_tester(R"(
		int_val := 1 + 2 + 3 + 4;
		float_val := 1.0 + 1.5 + 1.0;
		string_val := "test" + "ing" + ".";
		int_div := 1 / 2;
		float_div := 1.0 / 2.0;
	)");

	bexp_tester.add_test(
		+[](Type::Environment& env) -> int {
			auto* expected_10 = unboxed(env.m_scope->access("int_val"));
			auto* as_integer = dynamic_cast<Type::Integer*>(expected_10);
			assert(as_integer);
			assert(as_integer->m_value == 10);
			std::cout << "@@ Value is: " << as_integer->m_value << '\n';

			auto* expected_3_5 = unboxed(env.m_scope->access("float_val"));
			auto* as_float = dynamic_cast<Type::Float*>(expected_3_5);
			assert(as_float);
			assert(as_float->m_value == 3.5);
			std::cout << "@@ Value is: " << as_float->m_value << '\n';

			auto* expected_testing = unboxed(env.m_scope->access("string_val"));
			auto* as_string = dynamic_cast<Type::String*>(expected_testing);
			assert(as_string);
			assert(as_string->m_value == "testing.");
			std::cout << "@@ Value is: " << as_string->m_value << '\n';

			auto* expected_0 = unboxed(env.m_scope->access("int_div"));
			as_integer = dynamic_cast<Type::Integer*>(expected_0);
			assert(as_integer);
			assert(as_integer->m_value == 0);
			std::cout << "@@ Value is: " << as_integer->m_value << '\n';

			auto* expected_0_5 = unboxed(env.m_scope->access("float_div"));
			as_float = dynamic_cast<Type::Float*>(expected_0_5);
			assert(as_float);
			assert(as_float->m_value == 0.5);
			std::cout << "@@ Value is: " << as_float->m_value << '\n';

			std::cout << "@ line " << __LINE__ << ": Success\n";
			return 0;
		}
	);

	assert(0 == bexp_tester.execute());

	Tester function_return{R"(
		f := fn() {
			a := 1;
			b := 2;
			return a + b;
		};
	)"};

	function_return.add_test(+[](Type::Environment& env) -> int {
		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up
		TokenArray ta;
		auto top_level_call = parse_expression("f()", ta);

		Type::Value* returned = eval(top_level_call.m_result.get(), env);
		auto exitcode = (dynamic_cast<Type::Integer*>(returned)->m_value == 3) ? 0 : 1;
		if(!exitcode)
			std::cout << "@ line " << __LINE__ << ": Success\n";

		return exitcode;
	});

	assert(0 == function_return.execute());

	Tester function_captures{R"(
	K := fn (x) { return fn (y) { return x; }; };
	f := fn () {
		a := 42;
		b := 2;
		return K(a)(b);
	};
	)"};

	function_captures.add_test(+[](Type::Environment& env) -> int {
		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up
		TokenArray ta;
		auto top_level_call = parse_expression("f()", ta);

		Type::Value* returned = eval(top_level_call.m_result.get(), env);

		if (!returned)
			return 1;

		if (returned->type() != value_type::Integer)
			return 1;

		auto* as_int = static_cast<Type::Integer*>(returned);
		if (as_int->m_value != 42){
			std::cerr << "@@ Value is " << as_int->m_value << '\n';
			return 1;
		}

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == function_captures.execute());

	Tester short_functions {R"(
		K := fn (x) => fn (y) => x;
		S := fn(x) => fn(y) => fn(z) => x(z)(y(z));
		I := S(K)(K);
		__invoke := fn () => I(42);
	)"};

	short_functions.add_test(+[](Type::Environment& env)->int{
		TokenArray ta;
		auto top_level_call = parse_expression("__invoke()", ta);

		auto* result = eval(top_level_call.m_result.get(), env);
		if(!result)
			return 1;

		if(result->type() != value_type::Integer)
			return 2;

		auto* as_int = static_cast<Type::Integer*>(result);
		if(as_int->m_value != 42){
			return 3;
		}

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == short_functions.execute());

	Tester array_index {R"(
		cons := fn (l,r) {
			return array { l; r; };
		};

		car := fn(l) {
			return l[0];
		};

		cdr := fn(l) {
			return l[1];
		};

		__invoke := fn () {
			a := cons(0, cons(1, cons(2, cons(3, 1337))));
			return car(cdr(cdr(a)));
		};
	)"};

	array_index.add_test(+[](Type::Environment& env)->int{
		TokenArray ta;
		auto top_level_call = parse_expression("__invoke()", ta);

		auto* result = eval(top_level_call.m_result.get(), env);
		if(!result)
			return 1;

		if(result->type() != value_type::Integer)
			return 2;

		auto* as_int = static_cast<Type::Integer*>(result);
		if(as_int->m_value != 2){
			return 3;
		}

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == array_index.execute());

	Tester binary_search_tree{R"(
		Leaf := fn() => array { "Leaf" };
		Node := fn(x,l,r) => array { "Node"; x; l; r };

		insert := fn(tree, x) {
			ins := fn(tree, x, r) {
				if(tree[0] == "Leaf")
					return Node(x, Leaf(), Leaf());
				if(tree[1] == x)
					return tree;
				if(tree[1] < x)
					return Node(tree[1], tree[2], r(tree[3], x, r));
				if(x < tree[1])
					return Node(tree[1], r(tree[2], x, r), tree[3]);
			};
			return ins(tree, x, ins);
		};

		print_inorder := fn (tree) {
			pr := fn(t, p) {
				if(t[0] == "Leaf") return "";
				return p(t[2],p) + t[1] + p(t[3], p);
			};
			return pr(tree, pr);
		};

		__invoke := fn () {
			t0 := Leaf();
			t1 := insert(t0, "f");
			t2 := insert(t1, "b");
			t3 := insert(t2, "e");
			t4 := insert(t3, "c");
			t5 := insert(t4, "a");
			t6 := insert(t5, "g");
			t7 := insert(t6, "d");

			return print_inorder(t7);
		};
	)"};

	binary_search_tree.add_test(+[](Type::Environment& env)->int{
		TokenArray ta;
		auto top_level_call = parse_expression("__invoke()", ta);

		auto* result = eval(top_level_call.m_result.get(), env);
		if(!result)
			return 1;

		if(result->type() != value_type::String)
			return 2;

		auto* as_str = static_cast<Type::String*>(result);
		if(as_str->m_value != "abcdefg"){
			return 3;
		}

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == binary_search_tree.execute());

	Tester bool_and_null_literals{R"(
		litt := fn () { return true; };
		litf := fn () { return false; };
		nullv := fn () { return null; };
	)"};
	
	bool_and_null_literals.add_test(+[](Type::Environment& env)->int{
		{
			TokenArray ta;
			auto top_level_call = parse_expression("litt()", ta);

			auto* result = eval(top_level_call.m_result.get(), env);
			if(!result)
				return 1;

			if(result->type() != value_type::Boolean)
				return 2;

			auto* as_bool = static_cast<Type::Boolean*>(result);
			if(as_bool->m_value != true){
				return 3;
			}
		}
		{	
			TokenArray ta;
			auto top_level_call = parse_expression("litf()", ta);

			auto* result = eval(top_level_call.m_result.get(), env);
			if(!result)
				return 1;

			if(result->type() != value_type::Boolean)
				return 2;

			auto* as_bool = static_cast<Type::Boolean*>(result);
			if(as_bool->m_value != false){
				return 3;
			}
		}
		{	
			TokenArray ta;
			auto top_level_call = parse_expression("nullv()", ta);

			auto* result = eval(top_level_call.m_result.get(), env);
			if(!result)
				return 1;

			if(result->type() != value_type::Null)
				return 2;
		}

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == bool_and_null_literals.execute());


	Tester recursion{R"(
		fib := fn(n){
			if(n < 2) return n;
			return fib(n-1) + fib(n-2);
		};
		__invoke := fn() => fib(6);
	)"};

	recursion.add_test(+[](Type::Environment& env)->int{
		TokenArray ta;
		auto top_level_call = parse_expression("__invoke()", ta);

		auto* result = eval(top_level_call.m_result.get(), env);
		if(!result)
			return 1;

		if(result->type() != value_type::Integer)
			return 2;

		auto* as_int = static_cast<Type::Integer*>(result);
		if(as_int->m_value != 8){
			return 3;
		}

		std::cout << "@ line " << __LINE__ << ": Success\n";
		return 0;
	});

	assert(0 == recursion.execute());
}
