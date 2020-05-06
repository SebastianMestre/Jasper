#include <iostream>
#include <string>
#include <vector>

#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main() {
	std::vector<char> v;
	std::string s = R"(
	x : dec = 1.4;
	y : int = 3;
	z := fn () {
		a := 2;
		b : dec = (4 + 5) * 3 * 1 + 4.5 * 2;
		c := a;
		a + b;
		cmp : bool = a < c;
		
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
		sqrt5 := obt{x:=1;y:=2;} |> norm();
		sixteen := my_lib.times_pi(my_lib.five()) + 1;
	};

	names := dict {
		user0 := "peter";
		user1 := "joseph";
		user2 := "anne";
	};
)";

	for (char c : s) {
		v.push_back(c);
	}

	Lexer l;
	l.m_source = std::move(v);

	Parser p;
	p.m_lexer = &l;

	auto parse_result = p.parse_top_level();
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return 1;
	} 

	print(parse_result.m_result.get(), 1);

	auto* top_level = parse_result.m_result.get();
	assert(top_level->type() == ast_type::DeclarationList);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = {&gc, &scope};

	eval(top_level, env);

	auto* entry_point_ptr = env.m_scope->access("__invoke");
	if(!entry_point_ptr){
		std::cerr << "__invoke is not defined\n";
		return 1;
	}

	if(entry_point_ptr == nullptr){
		std::cerr << "entry point is nullptr: " << entry_point_ptr << "\n";
		return 1;
	}

	if(entry_point_ptr == gc.null()){
		std::cerr << "entry point is null: " << entry_point_ptr << "\n";
		return 1;
	}

	auto* entry_point = dynamic_cast<Type::Function*>(entry_point_ptr);
	if(!entry_point){
		std::cerr << "__invoke is not a function\n";
		return 1;
	}

	// entry_point->call(env);

	gc.run();

	return 0;
}
