#include <iostream>
#include <cassert>

#include "eval.hpp"
#include "parse.hpp"
#include "tester.hpp"
#include "token_array.hpp"
#include "value.hpp"
#include "typed_ast.hpp"
#include "typed_ast_type.hpp"
#include "test_utils.hpp"

void assert_equals(int expected, int received) {
	if (expected != received) {
		std::cout << "Error: expected " << expected << " but got " << received << std::endl;
		assert(0);
	}
}

int main() {
	using Test::Tester;
	using TypedAST::get_unique;

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

)"};

	monolithic_test.add_test(+[](Type::Environment& env) -> int {
		Assert::eval_expression("__invoke()", env);
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

	bexp_tester.add_tests( {
		+[](Type::Environment& env) -> int {
			return Assert::equals("int_val", 10, env);
		},

		+[](Type::Environment& env) -> int {
			return Assert::equals("float_val", 3.5, env);
		},

		+[](Type::Environment& env) -> int {
			return Assert::equals("string_val", "testing.", env);
		},

		+[](Type::Environment& env) -> int {
			return Assert::equals("int_div", 0, env);
		},

		+[](Type::Environment& env) -> int {
			return Assert::equals("float_div", 0.5, env);
		},
	} );

	assert(0 == bexp_tester.execute());

	Tester function_return{R"(
		f := fn() {
			a := 1;
			b := 2;
			return a + b;
		};
	)"};

	function_return.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("f()", 3, env);
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
		return Assert::equals("f()", 42, env);
	});

	assert(0 == function_captures.execute());

	Tester short_functions {R"(
		K := fn (x) => fn (y) => x;
		S := fn(x) => fn(y) => fn(z) => x(z)(y(z));
		I := S(K)(K);
		__invoke := fn () => I(42);
	)"};

	short_functions.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", 42, env);
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

	array_index.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", 2, env);
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

	binary_search_tree.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", "abcdefg", env);
	});

	assert(0 == binary_search_tree.execute());

	Tester bool_and_null_literals{R"(
		litt := fn () { return true; };
		litf := fn () { return false; };
		nullv := fn () { return null; };
	)"};
	
	bool_and_null_literals.add_tests({
		+[](Type::Environment& env) -> int {
			return Assert::is_true("litt()", env);
		},	

		+[](Type::Environment& env) -> int {
			return Assert::is_false("litf()", env);
		},

		+[](Type::Environment& env) -> int {
			return Assert::is_null("nullv()", env);
		}
	});
	
	assert(0 == bool_and_null_literals.execute());

	Tester recursion{R"(
		fib := fn(n){
			if(n < 2) return n;
			return fib(n-1) + fib(n-2);
		};
		__invoke := fn() => fib(6);
	)"};

	recursion.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", 8, env);
	});

	assert(0 == recursion.execute());
	
	Tester sum_first_integers{R"(
		__invoke := fn () {
			sum := 0;
			N := 16;
			for (i := 0; i < N; i = i + 1) {
				sum = sum + i;
			}
			return sum;
		};
	)"};

	sum_first_integers.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", 120, env);
	});
	
	assert(0 == sum_first_integers.execute());

	Tester native_array_append{R"(
		__invoke := fn () {
			A := array {};
			array_append(A, 10);
			return A;
		};
	)"};

	native_array_append.add_test(+[](Type::Environment& env)->int{
		auto* result = Assert::eval_expression("__invoke()", env);

		if(!result)
			return 1;

		if (result->type() != value_type::Array)
			return 2;

		auto* array = static_cast<Type::Array*>(result);
		if (array->m_value.size() != 1)
			return 3;

		if (array->m_value[0]->type() != value_type::Integer) {
			return 4;
		}

		auto* value = static_cast<Type::Integer*>(array->m_value[0]);
		if (value->m_value != 10)
			return 5;

		std::cout << "@ line " << __LINE__ << ": Success \n";
		return 0;
	});

	assert(0 == native_array_append.execute());

	Tester native_array_extend{R"(
		__invoke := fn () {
			A := array {};
			array_extend(A, array{10});
			return A;
		};
	)"};

	native_array_extend.add_test(+[](Type::Environment& env)->int{
		auto* result = Assert::eval_expression("__invoke()", env);

		if(!result)
			return 1;

		if (result->type() != value_type::Array)
			return 2;

		auto* array = static_cast<Type::Array*>(result);
		if (array->m_value.size() != 1)
			return 3;

		if (array->m_value[0]->type() != value_type::Integer)
			return 4;

		auto* value = static_cast<Type::Integer*>(array->m_value[0]);
		if (value->m_value != 10)
			return 5;

		std::cout << "@ line " << __LINE__ << ": Success \n";
		return 0;
	});

	assert(0 == native_array_extend.execute());

	Tester native_size{R"(
		__invoke := fn () {
			A := array {10;10};
			return size(A);
		};
	)"};

	native_size.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", 2, env);
	});

	assert(0 == native_size.execute());

	Tester native_array_join{R"(
		__invoke := fn () {
			A := array {10;10};
			return array_join(A, ",");
		};
	)"};

	native_array_join.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", "10,10", env);
	});

	assert(0 == native_array_join.execute());

	Tester compound_types{R"(
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
	)"};

	compound_types.add_test(+[](Type::Environment& env) -> int {
		TokenArray ta;
		auto top_level_call_ast = parse_expression("__invoke()", ta);
		auto top_level_call = get_unique(top_level_call_ast.m_result);

		auto* result = unboxed(eval(top_level_call.get(), env));
		if (!result)
			return 1;

		if (result->type() != value_type::Integer)
			return 2;

		auto* as_int = static_cast<Type::Integer*>(result);
		if (as_int->m_value != 4)
			return 3;

		std::cout << "@ line " << __LINE__ << ": Success \n";
		return 0;
	});

	assert(0 == compound_types.execute());

	Tester pizza_operator{R"(
		f := fn(x) => x + 7;
		__invoke := fn() => 6 |> f();
	)"};
	
	pizza_operator.add_test(+[](Type::Environment& env) -> int {
		return Assert::equals("__invoke()", 13, env);
	});

	assert_equals(0, pizza_operator.execute());
}
