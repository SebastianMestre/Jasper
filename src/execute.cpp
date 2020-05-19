#include "execute.hpp"

#include "captures.hpp"
#include "environment.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "parse.hpp"
#include "token_array.hpp"

int execute(std::string const& source, bool dump_ast, Runner* runner) {

	TokenArray ta;
	auto parse_result = parse_program(source, ta);
	if (not parse_result.ok()) {
		parse_result.m_error.print();
		return 1;
	}

	gather_captures(parse_result.m_result.get());

	if (dump_ast)
		print(parse_result.m_result.get(), 1);

	GarbageCollector::GC gc;
	Type::Scope scope;
	Type::Environment env = { &gc, &scope, &scope};

	auto* top_level = parse_result.m_result.get();
	if (top_level->type() != ast_type::DeclarationList)
		return 1;

	// TODO: put native functions in a better place
	env.declare(
	    "print",
	    env.new_native_function(
	        +[](Type::ArrayType v, Type::Environment& e) -> Type::Value* {
				Type::print(v[0]);
		        return e.null();
	        }));

	env.declare(
		"array_append",
		env.new_native_function(
			[](Type::ArrayType v, Type::Environment& e) -> Type::Value* {
				Type::Array* array = static_cast<Type::Array*>(v[0]);
				Type::Value* value = v[1];
				array->m_value.push_back(value);
				return array;
			}));

	eval(top_level, env);

	int runner_exit_code = runner(env);

	if(runner_exit_code)
		return runner_exit_code;

	gc.run();

	return 0;
}
