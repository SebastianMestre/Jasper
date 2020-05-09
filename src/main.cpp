#include <iostream>
#include <string>
#include <vector>

#include "environment.hpp"
#include "eval.hpp"
#include "execute.hpp"
#include "garbage_collector.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

int main() {

	std::string s = R"(
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

)";

	int exit_code = execute(s, true, [&](Type::Environment& env) {
		auto* entry_point_ptr = env.m_scope->access("__invoke");
		if (!entry_point_ptr) {
			std::cerr << "__invoke is not defined\n";
			return 1;
		}

		if (entry_point_ptr == nullptr) {
			std::cerr << "entry point is nullptr: " << entry_point_ptr << "\n";
			return 1;
		}

		if (entry_point_ptr == env.m_gc->null()) {
			std::cerr << "entry point is null: " << entry_point_ptr << "\n";
			return 1;
		}

		auto* entry_point = dynamic_cast<Type::Function*>(entry_point_ptr);
		if (!entry_point) {
			std::cerr << "__invoke is not a function\n";
			return 1;
		}

		// NOTE: We currently implement funcion evaluation in eval(ASTCallExpression)
		// this means we need to create a call expression node to run the program.
		// TODO: We need to clean this up

		{
			auto top_level_name = std::make_unique<ASTIdentifier>();
			top_level_name->m_text = "__invoke";

			auto top_level_call = std::make_unique<ASTCallExpression>();
			top_level_call->m_callee = std::move(top_level_name);
			top_level_call->m_args = std::make_unique<ASTArgumentList>();

			eval(top_level_call.get(), env);
		}

		return 0;
	});

	return exit_code;
}
