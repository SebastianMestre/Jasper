#include "match_identifiers.hpp"

#include <iostream>
#include <unordered_map>

#include "compile_time_environment.hpp"
#include "error_report.hpp"
#include "typed_ast.hpp"

#include <cassert>

namespace TypeChecker {

#define CHECK_AND_RETURN(expr)                                                 \
	{                                                                          \
		auto err = expr;                                                       \
		if (!err.ok())                                                         \
			return err;                                                        \
	}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::Declaration* ast, Frontend::CompileTimeEnvironment& env) {

	ast->m_surrounding_function = env.current_function();
	env.declare(ast->identifier_text(), ast);

	if (ast->m_value)
		CHECK_AND_RETURN(match_identifiers(ast->m_value, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::Identifier* ast, Frontend::CompileTimeEnvironment& env) {

	TypedAST::Declaration* declaration = env.access(ast->text());

	if (!declaration) {
		// TODO: clean up how we build error reports
		return {
		    "ERROR @ line " + std::to_string(ast->m_token->m_line0 + 1) +
		    " : accessed undeclared identifier '" + ast->text().str() + "'"};
	}

	ast->m_declaration = declaration;
	env.current_top_level_declaration()->m_references.insert(declaration);
	TypedAST::FunctionLiteral* surrounding_function =
	    declaration->m_surrounding_function;

	// dont capture globals
	if (surrounding_function) {
		for (int i = env.m_function_stack.size(); i--;) {
			auto* func = env.m_function_stack[i];
			if (func == surrounding_function)
				break;
			func->m_captures.insert(ast->text());
		}
	}

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::Block* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	for (auto& child : ast->m_body)
		CHECK_AND_RETURN(match_identifiers(child, env));
	env.end_scope();
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::IfElseStatement* ast, Frontend::CompileTimeEnvironment& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));

	if (ast->m_else_body)
		CHECK_AND_RETURN(match_identifiers(ast->m_else_body, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::CallExpression* ast, Frontend::CompileTimeEnvironment& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_callee, env));
	for (auto& arg : ast->m_args)
		CHECK_AND_RETURN(match_identifiers(arg, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::FunctionLiteral* ast, Frontend::CompileTimeEnvironment& env) {

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	int arg_count = ast->m_args.size();

	for (int i = 0; i < arg_count; ++i)
		env.declare(ast->m_args[i].identifier_text(), &ast->m_args[i]);

	// scan body
	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body);
	for (auto& child : body->m_body)
		CHECK_AND_RETURN(match_identifiers(child, env));

	env.end_scope();
	env.exit_function();

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::ArrayLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	for (auto& element : ast->m_elements)
		CHECK_AND_RETURN(match_identifiers(element, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::ForStatement* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();

	CHECK_AND_RETURN(match_identifiers(&ast->m_declaration, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_action, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));

	env.end_scope();
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::WhileStatement* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();

	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));

	env.end_scope();
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::ReturnStatement* ast, Frontend::CompileTimeEnvironment& env) {
	return match_identifiers(ast->m_value, env);
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::IndexExpression* ast, Frontend::CompileTimeEnvironment& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_callee, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_index, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::TernaryExpression* ast, Frontend::CompileTimeEnvironment& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_then_expr, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_else_expr, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::RecordAccessExpression* ast, Frontend::CompileTimeEnvironment& env) {
	return match_identifiers(ast->m_record, env);
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::DeclarationList* ast, Frontend::CompileTimeEnvironment& env) {
	for (auto& decl : ast->m_declarations) {
		env.declare(decl.identifier_text(), &decl);
		decl.m_surrounding_function = env.current_function();
	}

	for (auto& decl : ast->m_declarations) {
		env.enter_top_level_decl(&decl);

		if (decl.m_value)
			CHECK_AND_RETURN(match_identifiers(decl.m_value, env));

		env.exit_top_level_decl();
	}

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::StructExpression* ast, Frontend::CompileTimeEnvironment& env) {

	for (auto& type : ast->m_types)
		CHECK_AND_RETURN(match_identifiers(type, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::TypeTerm* ast, Frontend::CompileTimeEnvironment& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_callee, env));
	for (auto& arg : ast->m_args)
		CHECK_AND_RETURN(match_identifiers(arg, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(
    TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment& env) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return match_identifiers(static_cast<TypedAST::type*>(ast), env);

#define DO_NOTHING(type)                                                       \
	case TypedASTTag::type:                                                    \
		return {};

	switch (ast->type()) {
		DO_NOTHING(NumberLiteral);
		DO_NOTHING(IntegerLiteral);
		DO_NOTHING(StringLiteral);
		DO_NOTHING(BooleanLiteral);
		DO_NOTHING(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(FunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(IndexExpression);
		DISPATCH(CallExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(RecordAccessExpression);

		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}

#undef DO_NOTHING
#undef DISPATCH
	std::cerr << "INTERNAL ERROR: UNHANDLED CASE IN " << __PRETTY_FUNCTION__
	          << ": " << typed_ast_string[(int)ast->type()] << '\n';
	assert(0);
}

#undef CHECK_AND_RETURN

} // namespace TypeChecker
