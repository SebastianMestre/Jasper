#include "match_identifiers.hpp"

#include <unordered_map>
#include <iostream>

#include "compile_time_environment.hpp"
#include "typed_ast.hpp"
#include "typesystem_types.hpp"

#include <cassert>

namespace TypeChecker {

void match_identifiers(
    TypedAST::NumberLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_int();
}

void match_identifiers(
    TypedAST::StringLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_string();
}

void match_identifiers(
    TypedAST::BooleanLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_boolean();
}

void match_identifiers(
    TypedAST::NullLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_unit();
}

void match_identifiers(
    TypedAST::Declaration* ast, Frontend::CompileTimeEnvironment& env) {
#if DEBUG
	std::cerr << "Typechecking " << ast->identifier_text() << '\n';
#endif

	ast->m_surrounding_function = env.current_function();
	env.declare(ast->identifier_text(), ast);

	// this is where we implement let-polymorphism. TODO: refactor (duplication).
	if (ast->m_value)
		match_identifiers(ast->m_value.get(), env);

	MonoId mono = ast->m_value ? ast->m_value->m_value_type
	                           : env.m_typechecker.new_var();

	ast->m_decl_type = env.m_typechecker.m_core.generalize(mono);

#if DEBUG
	{
		std::cerr << "Type of " << ast->identifier_text() << " is:\n";
		auto poly = ast->m_decl_type;
		auto mono = env.m_typechecker.m_core.poly_data[poly].base;
		env.m_typechecker.m_core.print_type(mono);
	}
#endif
}

void match_identifiers(TypedAST::Identifier* ast, Frontend::CompileTimeEnvironment& env) {
	Frontend::Binding* binding = env.access_binding(ast->text());
	assert(binding && "accessed an undeclared identifier");

	// here we implement the [var] rule
	// TODO: refactor
	TypedAST::FunctionLiteral* surrounding_function = nullptr;
	MonoId mono = -1;
	if (binding->m_type == Frontend::BindingType::Declaration) {
		TypedAST::Declaration* declaration = binding->get_decl();

		assert(declaration);
		ast->m_declaration = declaration;

		surrounding_function = declaration->m_surrounding_function;
		mono = env.m_typechecker.m_core.inst_fresh(declaration->m_decl_type);
	} else {
		TypedAST::FunctionArgument& arg = binding->get_arg();

		// This is only used to build the top-level-declaration graph. since this
		// points to a function argument, it doesn't play a role in the graph, so
		// it's ok that it is nullptr.
		ast->m_declaration = nullptr;

		surrounding_function = binding->get_func();
		mono = arg.m_value_type;
	}

	assert(mono != -1);

	ast->m_value_type = mono;

	// dont capture globals
	if(surrounding_function) {
		for (int i = env.m_function_stack.size(); i--;) {
			auto* func = env.m_function_stack[i];
			if (func == surrounding_function)
				break;
			func->m_captures.insert(ast->text());
		}
	}

}

void match_identifiers(TypedAST::Block* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	for (auto& child : ast->m_body)
		match_identifiers(child.get(), env);
	env.end_scope();
}

void match_identifiers(TypedAST::IfStatement* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_condition.get(), env);
	match_identifiers(ast->m_body.get(), env);
}

void match_identifiers(TypedAST::CallExpression* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_callee.get(), env);
	for (auto& arg : ast->m_args)
		match_identifiers(arg.get(), env);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args)
		arg_types.push_back(arg->m_value_type);

	ast->m_value_type = env.m_typechecker.rule_app(std::move(arg_types), ast->m_callee->m_value_type);
}

void match_identifiers(
    TypedAST::FunctionLiteral* ast, Frontend::CompileTimeEnvironment& env) {

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	{
		// TODO: do something better, use the type hints
		ast->m_return_type = env.m_typechecker.new_var();

		int arg_count = ast->m_args.size();
		std::vector<MonoId> arg_types;

		for (int i = 0; i < arg_count; ++i){
			auto& arg_decl = ast->m_args[i];

			int mono = env.m_typechecker.new_var();

			arg_types.push_back(mono);
			arg_decl.m_value_type = mono;

			env.declare_arg(arg_decl.identifier_text(), ast, i);
		}

		// return type
		arg_types.push_back(ast->m_return_type);


		MonoId term_mono_id = env.m_typechecker.m_core.new_term(BuiltinType::Function, std::move(arg_types));
		ast->m_value_type = term_mono_id;
	}

	// scan body
	assert(ast->m_body->type() == ast_type::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body.get());
	for (auto& child : body->m_body)
		match_identifiers(child.get(), env);

	env.end_scope();
	env.exit_function();
}

void match_identifiers(TypedAST::ForStatement* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	match_identifiers(ast->m_declaration.get(), env);
	match_identifiers(ast->m_condition.get(), env);
	match_identifiers(ast->m_action.get(), env);
	match_identifiers(ast->m_body.get(), env);
	env.end_scope();
}

void match_identifiers(
    TypedAST::ReturnStatement* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_value.get(), env);

	auto mono = ast->m_value->m_value_type;
	auto func = env.current_function();
	env.m_typechecker.m_core.unify(func->m_return_type, mono);
}

void match_identifiers(TypedAST::IndexExpression* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_callee.get(), env);
	match_identifiers(ast->m_index.get(), env);
}

void match_identifiers(TypedAST::DeclarationList* ast, Frontend::CompileTimeEnvironment& env) {
	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		env.declare(d->identifier_text(), d);
		d->m_surrounding_function = env.current_function();
	}

	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());

		// this is where we implement let-polymorphism. TODO: refactor (duplication).
		if (d->m_value)
			match_identifiers(d->m_value.get(), env);

		MonoId mono = d->m_value ? d->m_value->m_value_type
			: env.m_typechecker.new_var();

		d->m_decl_type = env.m_typechecker.m_core.generalize(mono);

#if DEBUG
		{
			std::cerr << "@@ Type of " << d->identifier_text() << '\n';
			env.m_typechecker.m_core.print_type(mono);

			auto poly = d->m_decl_type;
			auto& poly_data = env.m_typechecker.m_core.poly_data[poly];

			std::cerr << "@@ Has " << poly_data.vars.size() << " variables\n";
		}
#endif
	}
}

void match_identifiers(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment& env) {
#define DISPATCH(type)                                                         \
	case ast_type::type:                                                       \
		return match_identifiers(static_cast<TypedAST::type*>(ast), env);

	// TODO: Compound literals
	switch (ast->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);

		DISPATCH(Declaration);
		DISPATCH(Identifier);
		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(IfStatement);
		DISPATCH(FunctionLiteral);
		DISPATCH(CallExpression);
		DISPATCH(ReturnStatement);
		DISPATCH(IndexExpression);
		DISPATCH(DeclarationList);
	case ast_type::BinaryExpression:
		assert(0);
		return;
	}

#undef DISPATCH
}

} // namespace TypeChecker
