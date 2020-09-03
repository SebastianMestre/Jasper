#include "match_identifiers.hpp"

#include <unordered_map>
#include <iostream>

#include "compile_time_environment.hpp"
#include "typed_ast.hpp"

#include <cassert>

namespace TypeChecker {

void match_identifiers(
    TypedAST::Declaration* ast, Frontend::CompileTimeEnvironment& env) {

	ast->m_surrounding_function = env.current_function();
	env.declare(ast->identifier_text(), ast);

	if (ast->m_value)
		match_identifiers(ast->m_value.get(), env);
}

void match_identifiers(TypedAST::Identifier* ast, Frontend::CompileTimeEnvironment& env) {
	Frontend::Binding* binding = env.access_binding(ast->text());
	assert(binding && "accessed an undeclared identifier");

	// TODO: refactor
	TypedAST::FunctionLiteral* surrounding_function = nullptr;
	if (binding->m_type == Frontend::BindingType::Declaration) {
		TypedAST::Declaration* declaration = binding->get_decl();

		assert(declaration);
		ast->m_declaration = declaration;

		env.current_top_level_declaration()->m_references.insert(declaration);

		surrounding_function = declaration->m_surrounding_function;
	} else {
		TypedAST::FunctionArgument& arg = binding->get_arg();

		// This is only used to build the top-level-declaration graph. since this
		// points to a function argument, it doesn't play a role in the graph, so
		// it's ok that it is nullptr.
		ast->m_declaration = nullptr;

		surrounding_function = binding->get_func();
	}

	// dont capture globals
	if (surrounding_function) {
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

void match_identifiers(TypedAST::IfElseStatement* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_condition.get(), env);
	match_identifiers(ast->m_body.get(), env);

	if (ast->m_else_body)
		match_identifiers(ast->m_else_body.get(), env);
}

void match_identifiers(TypedAST::CallExpression* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_callee.get(), env);
	for (auto& arg : ast->m_args)
		match_identifiers(arg.get(), env);
}

void match_identifiers(
    TypedAST::FunctionLiteral* ast, Frontend::CompileTimeEnvironment& env) {

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	int arg_count = ast->m_args.size();

	for (int i = 0; i < arg_count; ++i) {
		auto& arg_decl = ast->m_args[i];
		env.declare_arg(arg_decl.identifier_text(), ast, i);
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

void match_identifiers(TypedAST::WhileStatement* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	match_identifiers(ast->m_condition.get(), env);
	match_identifiers(ast->m_body.get(), env);
	env.end_scope();
}

void match_identifiers(
    TypedAST::ReturnStatement* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_value.get(), env);
}

void match_identifiers(TypedAST::IndexExpression* ast, Frontend::CompileTimeEnvironment& env) {
	match_identifiers(ast->m_callee.get(), env);
	match_identifiers(ast->m_index.get(), env);
}

void match_identifiers(
    TypedAST::DeclarationList* ast, Frontend::CompileTimeEnvironment& env) {
	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		env.declare(d->identifier_text(), d);
		d->m_surrounding_function = env.current_function();
	}

	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		env.enter_top_level_decl(d);

		if (d->m_value)
			match_identifiers(d->m_value.get(), env);

		env.exit_top_level_decl();
	}
}

void match_identifiers(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment& env) {
#define DISPATCH(type)                                                         \
	case ast_type::type:                                                       \
		return match_identifiers(static_cast<TypedAST::type*>(ast), env);

#define DO_NOTHING(type)                                                       \
	case ast_type::type:                                                       \
		return;

	// TODO: Compound literals
	switch (ast->type()) {
		DO_NOTHING(NumberLiteral);
		DO_NOTHING(IntegerLiteral);
		DO_NOTHING(StringLiteral);
		DO_NOTHING(BooleanLiteral);
		DO_NOTHING(NullLiteral);

		DISPATCH(Declaration);
		DISPATCH(Identifier);
		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(FunctionLiteral);
		DISPATCH(CallExpression);
		DISPATCH(ReturnStatement);
		DISPATCH(IndexExpression);
		DISPATCH(DeclarationList);
	case ast_type::BinaryExpression:
		assert(0);
		return;
	}

#undef DO_NOTHING
#undef DISPATCH
}

} // namespace TypeChecker
