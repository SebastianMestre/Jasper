#include <iostream>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"
#include "types.hpp"
#include "garbage_collector.hpp"

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

	parse_result.m_result->print();
	auto& top_level = static_cast<ASTDeclarationList&>(*parse_result.m_result);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env;

	env.m_gc = &gc;
	env.m_scope = &scope;

	top_level.eval(env);

	auto* entry_point = dynamic_cast<Type::Function*>(env.m_scope->access("__invoke"));
	if(!entry_point){
		std::cerr << "__invoke is not a function\n";
	}

	// entry_point->call(env);

	return 0;
}
