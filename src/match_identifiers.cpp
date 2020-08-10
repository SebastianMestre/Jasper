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
    TypedAST::Declaration* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_surrounding_function = env.current_function();
	env.declare(ast->identifier_text(), ast);

	// this is where we implement let-polymorphism. TODO: refactor (duplication).
	if (ast->m_value)
		match_identifiers(ast->m_value.get(), env);

	MonoId mono = ast->m_value ? ast->m_value->m_value_type
	                           : env.m_typechecker.new_var();

	ast->m_decl_type = env.m_typechecker.m_core.generalize(mono);
}

void match_identifiers(TypedAST::Identifier* ast, Frontend::CompileTimeEnvironment& env) {
	TypedAST::Declaration* declaration = env.access(ast->text());
	assert(declaration);
	ast->m_declaration = declaration;

	for(int i = env.m_function_stack.size(); i--;){
		auto* func = env.m_function_stack[i];

		if(func == declaration->m_surrounding_function)
			break;

		func->m_captures.insert(ast->text());
	}

	// TODO: should this be a Var instead of just an id copy?
	ast->m_value_type = env.m_typechecker.m_core.inst_fresh(declaration->m_decl_type);
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

void match_identifiers(TypedAST::FunctionLiteral* ast, Frontend::CompileTimeEnvironment& env) {

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	{
		// TODO: do something better, use the type hints

		int arg_count = ast->m_args.size();
		std::vector<MonoId> arg_types;

		// return type
		arg_types.push_back(env.m_typechecker.new_var());

		for (int i = 0; i < arg_count; ++i){
			auto decl = static_cast<TypedAST::Declaration*>(ast->m_args[i].get());

			int mono = env.m_typechecker.new_var();
			arg_types.push_back(mono);

			// don't generalize the variable: wrap it in a poly type but leave it
			// free, succeptible to unification inside the function body.
			// TODO: This is a hack. we should better differentiate between
			// function arguments and declarations
			decl->m_decl_type = env.m_typechecker.m_core.new_poly(mono, {});
			decl->m_surrounding_function = ast;

			env.declare(decl->identifier_text(), decl);
		}

		MonoId term_mono_id = env.m_typechecker.m_core.new_term(BuiltinType::Function, std::move(arg_types));
		ast->m_value_type = term_mono_id;
	}

	// declare arguments
	for (auto& decl : ast->m_args)
		match_identifiers(decl.get(), env);

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

void match_identifiers(TypedAST::ReturnStatement* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_value.get(), env);
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

		// this is where we implement let-polymorphism. TODO: refactor.
		if (d->m_value)
			match_identifiers(d->m_value.get(), env);

		MonoId mono = d->m_value ? d->m_value->m_value_type
			: env.m_typechecker.new_var();

		d->m_decl_type = env.m_typechecker.m_core.generalize(mono);
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
	case ast_type::NullLiteral:
	case ast_type::BooleanLiteral:
		return;
	}

#undef DISPATCH
}

} // namespace TypeChecker
