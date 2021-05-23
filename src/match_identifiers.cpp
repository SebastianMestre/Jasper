#include "match_identifiers.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>

#include "./log/log.hpp"
#include "./utils/interned_string.hpp"
#include "ast.hpp"
#include "error_report.hpp"
#include "symbol_table.hpp"
#include "token.hpp"

#include <cassert>

namespace Frontend {

#define CHECK_AND_RETURN(expr)                                                 \
	{                                                                          \
		auto err = expr;                                                       \
		if (!err.ok()) {                                                       \
			return err;                                                        \
		}                                                                      \
	}

#define CHECK_AND_WRAP(expr, wrap)                                             \
	{                                                                          \
		auto err = (expr);                                                     \
		if (!err.ok()) {                                                       \
			return {(wrap), {std::move(err)}};                                 \
		}                                                                      \
	}

[[nodiscard]] ErrorReport match_identifiers(AST::AST* ast, SymbolTable& env);

[[nodiscard]] ErrorReport match_identifiers(AST::Declaration* ast, SymbolTable& env) {

	ast->m_surrounding_function = env.current_function();
	ast->m_surrounding_seq_expr = env.current_seq_expr();
	env.declare(ast);

	if (ast->m_type_hint)
		CHECK_AND_RETURN(match_identifiers(ast->m_type_hint, env));

	if (ast->m_value) {
		CHECK_AND_WRAP(
		    match_identifiers(ast->m_value, env),
		    "While scanning declaration '" + ast->identifier_text().str() + "'");
	}

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::Identifier* ast, SymbolTable& env) {

	AST::Declaration* declaration = env.access(ast->text());

	if (!declaration) {
		// TODO: clean up how we build error reports
		auto token = ast->token();
		return make_located_error(
		    "accessed undeclared identifier '" + ast->text().str() + "'",
		    token->m_source_location.start);
	}

	ast->m_declaration = declaration;
	ast->m_surrounding_function = env.current_function();

	env.current_top_level_declaration()->m_references.insert(declaration);

	if (declaration->is_global()) {
		ast->m_origin = AST::Identifier::Origin::Global;
	} else if (declaration->m_surrounding_function != ast->m_surrounding_function) {
		ast->m_origin = AST::Identifier::Origin::Capture;
	} else {
		ast->m_origin = AST::Identifier::Origin::Local;
	}

	// dont capture globals
	if (!declaration->is_global()) {
		for (int i = env.m_function_stack.size(); i--;) {
			auto* func = env.m_function_stack[i];
			if (func == declaration->m_surrounding_function)
				break;
			func->m_captures.insert({ast->text(), {declaration}});
		}
	}

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::Block* ast, SymbolTable& env) {
	env.new_nested_scope();
	for (auto& child : ast->m_body)
		CHECK_AND_RETURN(match_identifiers(child, env));
	env.end_scope();
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::IfElseStatement* ast, SymbolTable& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));

	if (ast->m_else_body)
		CHECK_AND_RETURN(match_identifiers(ast->m_else_body, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::CallExpression* ast, SymbolTable& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_callee, env));
	for (auto& arg : ast->m_args)
		CHECK_AND_RETURN(match_identifiers(arg, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::FunctionLiteral* ast, SymbolTable& env) {

	ast->m_surrounding_function = env.current_function();

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	for (auto& arg : ast->m_args) {
		if (arg.m_type_hint)
			CHECK_AND_RETURN(match_identifiers(arg.m_type_hint, env));
		env.declare(&arg);
	}

	// scan body
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));

	env.end_scope();
	env.exit_function();

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::ArrayLiteral* ast, SymbolTable& env) {
	for (auto& element : ast->m_elements)
		CHECK_AND_RETURN(match_identifiers(element, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::WhileStatement* ast, SymbolTable& env) {
	env.new_nested_scope();

	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));

	env.end_scope();
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::ReturnStatement* ast, SymbolTable& env) {
	ast->m_surrounding_seq_expr = env.current_seq_expr();
	return match_identifiers(ast->m_value, env);
}

[[nodiscard]] ErrorReport match_identifiers(AST::IndexExpression* ast, SymbolTable& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_callee, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_index, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::TernaryExpression* ast, SymbolTable& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_condition, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_then_expr, env));
	CHECK_AND_RETURN(match_identifiers(ast->m_else_expr, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::AccessExpression* ast, SymbolTable& env) {
	return match_identifiers(ast->m_record, env);
}

[[nodiscard]] ErrorReport match_identifiers(AST::MatchExpression* ast, SymbolTable& env) {

	CHECK_AND_RETURN(match_identifiers(&ast->m_matchee, env));

	if (ast->m_type_hint)
		CHECK_AND_RETURN(match_identifiers(ast->m_type_hint, env));

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		env.new_nested_scope();

		CHECK_AND_RETURN(match_identifiers(&case_data.m_declaration, env));
		CHECK_AND_RETURN(match_identifiers(case_data.m_expression, env));

		env.end_scope();
	}

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::ConstructorExpression* ast, SymbolTable& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_constructor, env));

	for (auto& arg : ast->m_args)
		CHECK_AND_RETURN(match_identifiers(arg, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::SequenceExpression* ast, SymbolTable& env) {
	env.enter_seq_expr(ast);
	CHECK_AND_RETURN(match_identifiers(ast->m_body, env));
	env.exit_seq_expr();
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::DeclarationList* ast, SymbolTable& env) {
	for (auto& decl : ast->m_declarations) {
		env.declare(&decl);
		decl.m_surrounding_function = env.current_function();
	}

	for (auto& decl : ast->m_declarations) {
		env.enter_top_level_decl(&decl);

		if (decl.m_type_hint)
			CHECK_AND_RETURN(match_identifiers(decl.m_type_hint, env));

		if (decl.m_value)
			CHECK_AND_WRAP(
			    match_identifiers(decl.m_value, env),
			    "While scanning top level declaration '" +
			        decl.identifier_text().str() + "'");

		env.exit_top_level_decl();
	}

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::UnionExpression* ast, SymbolTable& env) {

	for (auto& type : ast->m_types)
		CHECK_AND_RETURN(match_identifiers(type, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::StructExpression* ast, SymbolTable& env) {

	for (auto& type : ast->m_types)
		CHECK_AND_RETURN(match_identifiers(type, env));

	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::TypeTerm* ast, SymbolTable& env) {
	CHECK_AND_RETURN(match_identifiers(ast->m_callee, env));
	for (auto& arg : ast->m_args)
		CHECK_AND_RETURN(match_identifiers(arg, env));
	return {};
}

[[nodiscard]] ErrorReport match_identifiers(AST::AST* ast, SymbolTable& env) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return match_identifiers(static_cast<AST::type*>(ast), env);

#define DO_NOTHING(type)                                                       \
	case ASTTag::type:                                                    \
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
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		DISPATCH(Block);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(UnionExpression);
		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}

#undef DO_NOTHING
#undef DISPATCH
	Log::fatal() << "(internal) Unhandled case in match_identifiers '" << ast_string[int(ast->type())] << "'";
}

#undef CHECK_AND_RETURN

} // namespace Frontend
