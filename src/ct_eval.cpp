#include "ct_eval.hpp"

#include "typechecker.hpp"
#include "typed_ast.hpp"

#include <iostream>

#include <cassert>

namespace TypeChecker {

// literals

Own<TypedAST::ArrayLiteral> ct_eval(Own<TypedAST::ArrayLiteral> ast, TypeChecker& tc) {
	for (auto&& element : ast->m_elements)
		element = ct_eval(std::move(element), tc);
	return ast;
}

// expressions

Own<TypedAST::FunctionLiteral> ct_eval(
    Own<TypedAST::FunctionLiteral> ast, TypeChecker& tc) {

	// TODO: type hints

	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body.get());
	for (auto&& child : body->m_body)
		child = ct_eval(std::move(child), tc);

	return ast;
}

Own<TypedAST::CallExpression> ct_eval(
    Own<TypedAST::CallExpression> ast, TypeChecker& tc) {

	ast->m_callee = ct_eval(std::move(ast->m_callee), tc);

	for (auto&& arg : ast->m_args)
		arg = ct_eval(std::move(arg), tc);

	return ast;
}

Own<TypedAST::IndexExpression> ct_eval(Own<TypedAST::IndexExpression> ast, TypeChecker& tc) {
	ast->m_callee = ct_eval(std::move(ast->m_callee), tc);
	ast->m_index = ct_eval(std::move(ast->m_index), tc);

	return ast;
}

Own<TypedAST::TernaryExpression> ct_eval(Own<TypedAST::TernaryExpression> ast, TypeChecker& tc) {
	ast->m_condition = ct_eval(std::move(ast->m_condition), tc);
	ast->m_then_expr = ct_eval(std::move(ast->m_then_expr), tc);
	ast->m_else_expr = ct_eval(std::move(ast->m_else_expr), tc);
	return ast;
}

Own<TypedAST::RecordAccessExpression> ct_eval(Own<TypedAST::RecordAccessExpression> ast, TypeChecker& tc) {
	ast->m_record = ct_eval(std::move(ast->m_record), tc);
	return ast;
}

// statements

Own<TypedAST::Block> ct_eval(Own<TypedAST::Block> ast, TypeChecker& tc) {
	for (auto&& child : ast->m_body)
		child = ct_eval(std::move(child), tc);
	return ast;
}

Own<TypedAST::IfElseStatement> ct_eval(Own<TypedAST::IfElseStatement> ast, TypeChecker& tc) {
	ast->m_condition = ct_eval(std::move(ast->m_condition), tc);
	ast->m_body = ct_eval(std::move(ast->m_body), tc);
	if (ast->m_else_body)
		ast->m_else_body = ct_eval(std::move(ast->m_else_body), tc);
	return ast;
}

Own<TypedAST::ForStatement> ct_eval(Own<TypedAST::ForStatement> ast, TypeChecker& tc) {
	ast->m_declaration = ct_eval(std::move(ast->m_declaration), tc);
	ast->m_condition = ct_eval(std::move(ast->m_condition), tc);
	ast->m_action = ct_eval(std::move(ast->m_action), tc);
	ast->m_body = ct_eval(std::move(ast->m_body), tc);
	return ast;
}

Own<TypedAST::WhileStatement> ct_eval(Own<TypedAST::WhileStatement> ast, TypeChecker& tc) {
	ast->m_condition = ct_eval(std::move(ast->m_condition), tc);
	ast->m_body = ct_eval(std::move(ast->m_body), tc);
	return ast;
}

Own<TypedAST::ReturnStatement> ct_eval(Own<TypedAST::ReturnStatement> ast, TypeChecker& tc) {
	ast->m_value = ct_eval(std::move(ast->m_value), tc);
	return ast;
}

// declarations

Own<TypedAST::Declaration> ct_eval(Own<TypedAST::Declaration> ast, TypeChecker& tc) {
	ast->m_value = ct_eval(std::move(ast->m_value), tc);
	return ast;
}

TypeFunctionId type_func_from_ast(TypedAST::TypedAST* ast, TypeChecker& tc) {
	assert(tc.m_core.m_meta_core.find(ast->m_meta_type) == tc.meta_typefunc());
	if(ast->type() == TypedASTTag::Identifier){
		auto as_id = static_cast<TypedAST::Identifier*>(ast);
		auto decl = as_id->m_declaration;
		auto value = static_cast<TypedAST::TypeFunctionHandle*>(decl->m_value.get());
		return value->m_value;
	} else {
		assert(0);
	}
}

Own<TypedAST::DeclarationList> ct_eval(Own<TypedAST::DeclarationList> ast, TypeChecker& tc) {
	for (auto& decl : ast->m_declarations) {
		if (tc.m_core.m_meta_core.find(decl->m_meta_type) == tc.meta_typefunc()) {
			// put a dummy typefunc var where required.
			auto handle = std::make_unique<TypedAST::TypeFunctionHandle>();
			handle->m_value = tc.m_core.m_tf_core.new_var();
			handle->m_syntax = std::move(decl->m_value);
			decl->m_value = std::move(handle);
		}
	}

	for (auto& decl : ast->m_declarations) {
		if (tc.m_core.m_meta_core.find(decl->m_meta_type) == tc.meta_typefunc()) {
			auto handle =
			    static_cast<TypedAST::TypeFunctionHandle*>(decl->m_value.get());
			TypeFunctionId tf = type_func_from_ast(handle->m_syntax.get(), tc);
			tc.m_core.m_tf_core.unify(tf, handle->m_value);
		} else {
			decl->m_value = ct_eval(std::move(decl->m_value), tc);
		}
	}

	return ast;
}

Own<TypedAST::TypedAST> ct_eval(Own<TypedAST::TypedAST> ast, TypeChecker& tc) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return ct_eval(                                                        \
		    Own<TypedAST::type> {static_cast<TypedAST::type*>(ast.release())}, tc)

#define RETURN(type)                                                           \
	case TypedASTTag::type:                                                    \
		return ast;

	switch (ast->type()) {
		RETURN(IntegerLiteral);
		RETURN(NumberLiteral);
		RETURN(BooleanLiteral);
		RETURN(StringLiteral);
		RETURN(NullLiteral);
		DISPATCH(ArrayLiteral);

		RETURN(Identifier);
		DISPATCH(FunctionLiteral);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(RecordAccessExpression);

		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);
	}

	std::cerr << "Unhandled case in " << __PRETTY_FUNCTION__ << " : "
	          << typed_ast_string[int(ast->type())] << "\n";
	assert(0);

#undef DISPATCH
#undef RETURN
}

} // namespace TypeChecker
