#include "match_identifiers.hpp"

#include <unordered_map>
#include <iostream>

#include "typed_ast.hpp"
#include "compile_time_environment.hpp"

#include <cassert>

namespace TypeChecker {

void match_identifiers(TypedAST::NumberLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_int();
}

void match_identifiers(TypedAST::StringLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_string();
}

void match_identifiers(TypedAST::Declaration* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_surrounding_function = env.current_function();
	env.declare(ast->identifier_text(), ast);
	if (ast->m_value) {
		match_identifiers(ast->m_value.get(), env);
		// TODO: should this be a Var instead of just an id copy?
		ast->m_decl_type = env.m_typechecker.m_core.new_poly(ast->m_value->m_value_type, {});
	} else {
		ast->m_decl_type = env.m_typechecker.m_core.new_poly(env.m_typechecker.new_var(), {});
	}
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
	// TODO: do something better, use the type hints
	ast->m_value_type = env.m_typechecker.new_var();

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

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
		if (d->m_value) {
			match_identifiers(d->m_value.get(), env);
			d->m_decl_type
			    = env.m_typechecker.m_core.generalize(d->m_value->m_value_type);
		} else {
			// TODO SPEED: dont create a variable and then generalize it, create a
			// generalized variable directly
			d->m_decl_type
			    = env.m_typechecker.m_core.generalize(env.m_typechecker.new_var());
		}
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
