#include <iostream>
#include <cassert>
#include <memory>

#include "../interpreter/environment_fwd.hpp"
#include "../interpreter/execute.hpp"
#include "test_utils.hpp"
#include "tester.hpp"

void interpreter_tests(Test::Tester& tests) {
	using TestCase = Test::InterpreterTestSet;
	using Testers = std::vector<Test::Interpret>;

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			x : dec = 1.4;
			y : int = 3;
			z := fn () {
				a := 2;
				b : dec = (4 + 5) * 3 * 1 + 4.5 * 2;
				c := a;
				a + b;
				cmp : bool = a < c;

				sqrt5 := obt{x:=1;y:=2;} |> norm();
				// sixteen := my_lib.times_pi(my_lib.five()) + 1;
			};

			y := fn () {
				print(1,z(),4);
			};

			w := fn (a,b:int,c){
				for(i := b; i < 3; i = i + 1)
					for(j := i + 1; j < 9; j = j + 1) {
						if (j < 9)
							return j;
						j = 2*j;
					}
				return 0;
			};

			// f := obt {
				// greeting := "Hello, ";
				// __invoke := fn (name : string) {
					// print(greeting + name);
				// };
			// };

			funct := fn (name : string) {};

			sqrt := fn (x) { return (x+1) * 0.5; };

			norm := fn(p){
				// f.greeting = "Hey, ";
				// f(" Sailor!");
				return sqrt(p.x * p.x + p.y * p.y);
			};

			my_lib := dict {
				five := fn() { return 6; };
				times_pi := fn(x) { return x * 3; };
			};

			__invoke := fn () {
				
			};

			names := dict {
				user0 := "peter";
				user1 := "joseph";
				user2 := "anne";
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			eval_expression("__invoke()", env);
			return exit_status_type::Ok;
		})
	);
	*/

	tests.add_test(
		std::make_unique<TestCase>(R"(
			int_val := 1 + 2 + 3 + 4;
			float_val := 1.0 + 1.5 + 1.0;
			string_val := "test" + "ing" + ".";
			int_div := 1 / 2;
			float_div := 1.0 / 2.0;
		)",
		Testers{
			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("int_val", env), 10);
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("float_val", env), 3.5);
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("string_val", env), "testing.");
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("int_div", env), 0);
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("float_div", env), 0.5);
			},
		})
	);

	tests.add_test(
		std::make_unique<TestCase>(R"(
			f := fn() {
				a := 1;
				b := 2;
				return a + b;
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("f()", env), 3);
		})
	);

	tests.add_test(
		std::make_unique<TestCase>(R"(
			K := fn (x) { return fn (y) { return x; }; };
			f := fn () {
				a := 42;
				b := 2;
				return K(a)(b);
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("f()", env), 42);
		})
	);

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			K := fn (x) => fn (y) => x;
			S := fn(x) => fn(y) => fn(z) => x(z)(y(z));
			I := S(K)(K);
			__invoke := fn () => I(42);
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 42);
		})
	);
	*/

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
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
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 2);
		})
	);
	*/

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
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
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), "abcdefg");
		})
	);
	*/

	tests.add_test(
		std::make_unique<TestCase>(R"(
			litt := fn () { return true; };
			litf := fn () { return false; };
			nullv := fn () { return null; };
		)",
		Testers{
			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::is_true(eval_expression("litt()", env));
			},	

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::is_false(eval_expression("litf()", env));
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::is_null(eval_expression("nullv()", env));
			}
		})
	);
	
	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			fib := fn(n){
				if(n < 2) return n;
				return fib(n-1) + fib(n-2);
			};
			__invoke := fn() => fib(6);
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 8);
		})
	);
	*/

	tests.add_test(
		std::make_unique<TestCase>(R"(
			__invoke := fn () {
				sum := 0;
				N := 16;
				for (i := 0; i < N; i = i + 1) {
					sum = sum + i;
				}
				return sum;
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 120);
		})
	);
	
	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			__invoke := fn () {
				A := array {};
				array_append(A, 10);
				return A;
			};
		)",
		Testers{
			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::array_of_size(eval_expression("__invoke()", env), 1);
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("__invoke()[0]", env), 10);
			}
		})
	);
	*/

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			__invoke := fn () {
				A := array {};
				array_extend(A, array{10});
				return A;
			};
		)",
		Testers{
			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::array_of_size(eval_expression("__invoke()", env), 1);
			},

			+[](Interpreter::Environment& env) -> exit_status_type {
				return Assert::equals(eval_expression("__invoke()[0]", env), 10);
			}
		})
	);
	*/

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			__invoke := fn () {
				A := array {10;10};
				return size(A);
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 2);
		})
	);
	*/

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			__invoke := fn () {
				A := array {10;10};
				return array_join(A, ",");
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), "10,10");
		})
	);
	*/

	/*
	tests.add_test(
		std::make_unique<TestCase>(R"(
			// TODO: fix inability to use keyword 'array' and others in types
			first_arr := fn(arr : Array(<Array(<Int>)>)) => arr[0];
			first_int := fn(arr : Array(<Ant>)) => arr[0];
			__invoke := fn(){
				mat := array {
					array { 4; 5; 6; };
					array { 1; 2; 3; };
				};
				arr := first_arr(mat);
				val := first_int(arr);
				return val;
			};
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 4);
		})
	);
	*/

	tests.add_test(
		std::make_unique<TestCase>(R"(
			f := fn(x) => x + 7;
			__invoke := fn() => 6 |> f();
		)",
		+[](Interpreter::Environment& env) -> exit_status_type {
			return Assert::equals(eval_expression("__invoke()", env), 13);
		})
	);

}

int main() {
	Test::Tester tests;
	interpreter_tests(tests);
	tests.execute();
}
