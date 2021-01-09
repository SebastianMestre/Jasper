#include <cassert>
#include <iostream>
#include <memory>

#include "../algorithms/tarjan_solver.hpp"
#include "../interpreter/environment_fwd.hpp"
#include "../interpreter/execute.hpp"
#include "../utils/string_set.hpp"
#include "test_status_tag.hpp"
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
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			eval_expression("__invoke()", env);
			return ExitStatusTag::Ok;
		})
	);
	*/


	tests.add_test(std::make_unique<TestCase>(
	    R"(
			int_val := 1 + 2 + 3 + 4;
			float_val := 1.0 + 1.5 + 1.0;
			string_val := "test" + "ing" + ".";
			int_div := 1 / 2;
			float_div := 1.0 / 2.0;
		)",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("int_val", env), 10);
	        },

	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("float_val", env), 3.5);
	        },

	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(
		            eval_expression("string_val", env), "testing.");
	        },

	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("int_div", env), 0);
	        },

	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("float_div", env), 0.5);
	        },
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			f := fn() {
				a := 1;
				b := 2;
				return a + b;
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("f()", env), 3);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			K := fn (x) { return fn (y) { return x; }; };
			f := fn () {
				a := 42;
				b := 2;
				return K(a)(b);
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("f()", env), 42);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			// K : forall a b. a -> b -> a
			K := fn (x) => fn (y) => x;
			// S : forall a b c. (a->b->c) -> (a->b) -> a -> c
			S := fn(x) => fn(y) => fn(z) => x(z)(y(z));
			// I : foral a. a -> a
			I := S(K)(K);
			__invoke := fn () => I(42);
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 42);
	    }));

	/*
	tests.add_test( // HEADS UP : this test won't work because it's ill typed
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
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("__invoke()", env), 2);
		})
	);
	*/

	/* // HEADS UP : this test won't work because it's ill typed
	tests.add_test(
		std::make_unique<TestCase>(R"(
			Leaf := fn() => array { "Leaf" };
			Node := fn(x,l,r) => array { "Node"; x; l; r };

			insert := fn(tree, x) {
				if(tree[0] == "Leaf")
					return Node(x, Leaf(), Leaf());
				if(tree[1] == x)
					return tree;
				if(tree[1] < x)
					return Node(tree[1], tree[2], insert(tree[3], x));
				if(x < tree[1])
					return Node(tree[1], insert(tree[2], x), tree[3]);
			};

			print_inorder := fn (tree) {
				if(t[0] == "Leaf") return "";
				return print_inorder(t[2],p) + t[1] + print_inorder(t[3], p);
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
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("__invoke()", env), "abcdefg");
		})
	);
	*/

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			litt := fn () { return true; };
			litf := fn () { return false; };
			nullv := fn () { return null; };
		)",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_true(eval_expression("litt()", env));
	        },

	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_false(eval_expression("litf()", env));
	        },

	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_null(eval_expression("nullv()", env));
	        }}));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			fib := fn(n){
				if(n < 2) return n;
				return fib(n-1) + fib(n-2);
			};
			__invoke := fn() => fib(6);
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 8);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn () {
				sum := 0;
				N := 16;
				for (i := 0; i < N; i = i + 1) {
					sum = sum + i;
				}
				return sum;
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 120);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn () {
				A := array {};
				array_append(A, 10);
				return A;
			};
		)",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::array_of_size(eval_expression("__invoke()", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("__invoke()[0]", env), 10);
	        }}));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn () {
				A := array {};
				array_extend(A, array{10});
				return A;
			};
		)",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::array_of_size(eval_expression("__invoke()", env), 1);
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("__invoke()[0]", env), 10);
	        }}));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn () {
				A := array {10;10};
				return size(A);
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 2);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn () {
				A := array {10;10};
				return array_join(A, ",");
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), "10,10");
	    }));

	/* // HEADS UP : this test won't work because we don't
	 * know how to take source type hints into account
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
		+[](Interpreter::Interpreter& env) -> ExitStatusTag {
			return Assert::equals(eval_expression("__invoke()", env), 4);
		})
	);
	*/

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			f := fn(x) => x + 7;
			__invoke := fn() => 6 |> f();
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 13);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn() {
				i := 0; sum := 0;

				while(i < 16) {
					sum = sum + i;
					i = i + 1;
				}

				return sum;
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 120);
	    }));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn(){
				return even(11);
			};

			even := fn(x) {
				if(x == 0) return true;
				return odd(x - 1);
			};

			odd := fn(x) {
				if(x == 0) return false;
				return even(x - 1);
			};
		)",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_false(eval_expression("__invoke()", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_true(eval_expression("odd(15)", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_true(eval_expression("even(80)", env));
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::is_false(eval_expression("odd(18)", env));
	        }}));

	tests.add_test(std::make_unique<TestCase>(
	    R"(
			__invoke := fn() {
				i := 2;
				if (i == 1)
					return false;
				else if (i == 3)
					return false;
				else if (i == 2)
					return true;
				return false;
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::is_true(eval_expression("__invoke()", env));
	    }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			__invoke := fn() {
				i := 0;
				return (if i == 1 then     0 else 1) +
				       (if i == 0 then i + 1 else 0);
			};
		)",
	    +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 2);
	    }));

	// we really only care that it typechecks, but might as
	// well throw a whole batch of stuff at it.
	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			__invoke := fn() {
				fib:= fn(n) => (if n < 2
					then n
					else fib(n-1) + fib(n-2));
				return fib(3);
			};
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 2);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			f := fn(x) {
				x.a = "a";
				return x.a;
			};
			__invoke := fn() => 0;
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 0);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			cat := fn(a,c,d) => fn(b) =>
				a + b + c + d;

			__invoke := fn() =>
				cat("A","C","D")("B");
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), "ABCD");
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			val := struct {
				a : string(<>);
				b : string(<>);
				c : string(<>);
			};
			test1 := fn () {
				v := val(<>){ "A"; "B"; "C"; };
				return v.a + v.b + v.c;
			};
			test2 := fn () {
				v := val(<>){ "A"; "B"; "C"; };
				return v.a + v.b + v.a;
			};
		)",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("test1()", env), "ABC");
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(eval_expression("test2()", env), "ABA");
		}}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			a : int(<>) = 1;
			b : string(<>) = "hello";
			c : array(< int(<>) >) = array { 1; 2; };

			__invoke := fn() {
				d : array(< string(<>) >) = array { "hello"; ","; "world"; };
				e : boolean(<>) = true;
				return d;
			};
		)",
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
	        }));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			typefunc := struct { x : int(<>); };
			other_typefunc := typefunc;
			other_other_typefunc := other_typefunc;

			mono := other_other_typefunc(<>);
			other_mono := mono;
			other_other_mono := other_mono;

			__invoke := fn() => other_other_mono { 10 };
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return ExitStatusTag::Ok;
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		list := struct {
			load : int(<>);
			next : list(<>);
		};
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return ExitStatusTag::Ok;
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
			a := c;
			b := a.id { 10 };
			c := union { id : int(<>); }(<>);
			__invoke := fn() => 0;
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 0);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"jp(
			b := union {
				str : string(<>);
				num : int(<>);
			}(<>);

			serialize := fn ( x ) {
				y := match(x) {
					str { s } => s;
					num { n } => "(number)";
				};
				return y;
			};

			from_string := fn(str) => b.str { str };
			from_number := fn(num) => b.num { num };
			__invoke := fn() {
			};
		)jp",
	    Testers {
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(
		            eval_expression("serialize(from_number(10))", env), "(number)");
	        },
	        +[](Interpreter::Interpreter& env) -> ExitStatusTag {
		        return Assert::equals(
		            eval_expression("serialize(from_string(\"xxx\"))", env), "xxx");
	        }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		tree := union {
			leaf : int(<>);
			node : tree_node(<>);
		};

		tree_node := struct {
			left : tree(<>);
			value : int(<>);
			right : tree(<>);
		};

		__invoke := fn() {
			x : tree(<>) = tree(<>).leaf {1};
			y : tree(<>) = tree(<>).node {
				tree_node(<>) {x; 2; x}
			};

			node_value := fn(node) => match(node) {
				leaf { i : int(<>) } => i;
				node { n : tree_node(<>) } => n.value;
			};

			return node_value(x) + node_value(y);
		};
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 3);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		f := fn (error_number : int(<>)) =>
			fn(x : either(<>)) => match(x) {
				left { i } => i;
				right { s } => error_number;
			};

		either := union {
			left : int(<>);
			right : string(<>);
		};

		__invoke := fn() {
			x := either(<>).left{ 1 };
			y := either(<>).right{ "error" };

			def := f(10);
			return def(x) + def(y);
		};
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 11);
	    }}));


	// Testing for a bug where we could not capture the inner variable that gets
	// bound in a match expression
	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"jp(
		A := union { X : int(<>); };
		__invoke := fn() {
			outter := A(<>).X{ 10 };
			k := match(outter) {
				X { inner } => fn() => inner;
			};
			return k();
		};
		)jp",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 10);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		// AST for a simple language
		expr := union {
			add : two_exprs(<>);
			val : int(<>);
		};

		two_exprs := struct {
			left : expr(<>);
			right : expr(<>);
		};

		// continuation passing style!
		eval := fn(e, c) => match(e : expr(<>)) {
			val { v } => c(v);
			add { v } => eval(v.left,  fn(x) =>
			             eval(v.right, fn(y) =>
			             c(x + y)));
		};

		__invoke := fn() => eval(
			expr(<>).add{
				two_exprs(<>){
					expr(<>).val{ 10 };
					expr(<>).val{ 32 };
				};
			}, fn(x) => x);
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 42);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		__invoke := fn() => seq {
			return 21 * 2;
		};
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 42);
	    }}));

	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		__invoke := fn() {
			arr := array { 0; 1; 2; 3; 4; };
			arr[0] = 1;
			arr[4] = 5;
			return arr[0] + arr[1] + arr[2] + arr[3] + arr[4];
		};
		)",
	    Testers {+[](Interpreter::Interpreter& env) -> ExitStatusTag {
		    return Assert::equals(eval_expression("__invoke()", env), 12);
	    }}));
	
	tests.add_test(std::make_unique<Test::InterpreterTestSet>(
	    R"(
		a := 2 - 1;
		b := 2 -1;
		c := 2-1;
		d := 2- 1;
		e := -1;
		f := -1.1;
		g := +1;
		h := +1.1;
		__invoke := fn() => 0;
		)",
	    Testers {
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
