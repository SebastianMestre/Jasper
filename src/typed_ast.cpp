#include <cassert>
#include <iostream>

#include "ast.hpp"
#include "typed_ast.hpp"
#include "typedefs.hpp"

namespace TypedAST {

Own<TypedAST> convertAST(AST::IntegerLiteral* ast) {
	auto typed_integer = std::make_unique<IntegerLiteral>();
	typed_integer->m_token = ast->m_token;
	return typed_integer;
}

Own<TypedAST> convertAST(AST::NumberLiteral* ast) {
	auto typed_number = std::make_unique<NumberLiteral>();
	typed_number->m_token = ast->m_token;
	return typed_number;
}

Own<TypedAST> convertAST(AST::StringLiteral* ast) {
	auto typed_string = std::make_unique<StringLiteral>();
	typed_string->m_token = ast->m_token;
	return typed_string;
}

Own<TypedAST> convertAST(AST::BooleanLiteral* ast) {
	auto typed_boolean = std::make_unique<BooleanLiteral>();
	typed_boolean->m_token = ast->m_token;
	return typed_boolean;
}

Own<TypedAST> convertAST(AST::NullLiteral* ast) {
	return std::make_unique<NullLiteral>();
}

Own<TypedAST> convertAST(AST::ArrayLiteral* ast) {
	auto typed_array = std::make_unique<ArrayLiteral>();

	for (auto& element : ast->m_elements) {
		typed_array->m_elements.push_back(convertAST(element.get()));
	}

	return typed_array;
}

Own<TypedAST> convertAST(AST::DictionaryLiteral* ast) {
	auto typed_dict = std::make_unique<DictionaryLiteral>();

	for (auto& element : ast->m_body) {
		typed_dict->m_body.push_back(convertAST(element.get()));
	}

	return typed_dict;
}

Own<TypedAST> convertAST(AST::FunctionLiteral* ast) {
	auto typed_function = std::make_unique<FunctionLiteral>();

	for (auto& arg : ast->m_args) {
		assert(arg->type() == ASTTag::Declaration);
		auto* decl = static_cast<AST::Declaration*>(arg.get());

		typed_function->m_args.push_back({decl->m_identifier_token});
	}

	typed_function->m_body = convertAST(ast->m_body.get());

	return typed_function;
}

Own<TypedAST> convertAST(AST::DeclarationList* ast) {
	auto typed_declist = std::make_unique<DeclarationList>();

	for (auto& declaration : ast->m_declarations) {
		typed_declist->m_declarations.push_back(convertAST(declaration.get()));
	}

	return typed_declist;
}

Own<TypedAST> convertAST(AST::Declaration* ast) {
	auto typed_dec = std::make_unique<Declaration>();

	typed_dec->m_identifier_token = ast->m_identifier_token;
	// TODO: handle type hint
	if (ast->m_value)
		typed_dec->m_value = convertAST(ast->m_value.get());

	return typed_dec;
}

Own<TypedAST> convertAST(AST::Identifier* ast) {
	auto typed_id = std::make_unique<Identifier>();
	typed_id->m_token = ast->m_token;
	return typed_id;
}

Own<TypedAST> convertAST(AST::CallExpression* ast) {
	auto typed_ce = std::make_unique<CallExpression>();

	for (auto& arg : ast->m_args) {
		typed_ce->m_args.push_back(convertAST(arg.get()));
	}

	typed_ce->m_callee = convertAST(ast->m_callee.get());

	return typed_ce;
}

Own<TypedAST> convertAST(AST::IndexExpression* ast) {
	auto typed_index = std::make_unique<IndexExpression>();

	typed_index->m_callee = convertAST(ast->m_callee.get());
	typed_index->m_index = convertAST(ast->m_index.get());

	return typed_index;
}

Own<TypedAST> convertAST(AST::TernaryExpression* ast) {
	auto typed_ternary = std::make_unique<TernaryExpression>();

	typed_ternary->m_condition = convertAST(ast->m_condition.get());
	typed_ternary->m_then_expr = convertAST(ast->m_then_expr.get());
	typed_ternary->m_else_expr = convertAST(ast->m_else_expr.get());

	return typed_ternary;
}

Own<TypedAST> convertAST(AST::RecordAccessExpression* ast) {
	auto typed_ast = std::make_unique<RecordAccessExpression>();

	// TODO: this line is extremely disgusting
	typed_ast->m_member = Own<Identifier>(
	    static_cast<Identifier*>(convertAST(ast->m_member.get()).release()));
	typed_ast->m_record = convertAST(ast->m_record.get());

	return typed_ast;
}

Own<TypedAST> convertAST(AST::Block* ast) {
	auto typed_block = std::make_unique<Block>();

	for (auto& element : ast->m_body) {
		typed_block->m_body.push_back(convertAST(element.get()));
	}

	return typed_block;
}

Own<TypedAST> convertAST(AST::ReturnStatement* ast) {
	auto typed_rs = std::make_unique<ReturnStatement>();

	typed_rs->m_value = convertAST(ast->m_value.get());

	return typed_rs;
}

Own<TypedAST> convertAST(AST::IfElseStatement* ast) {
	auto typed_if_else = std::make_unique<IfElseStatement>();

	typed_if_else->m_condition = convertAST(ast->m_condition.get());
	typed_if_else->m_body = convertAST(ast->m_body.get());

	if (ast->m_else_body)
		typed_if_else->m_else_body = convertAST(ast->m_else_body.get());

	return typed_if_else;
}

Own<TypedAST> convertAST(AST::ForStatement* ast) {
	auto typed_for = std::make_unique<ForStatement>();

	typed_for->m_declaration = convertAST(ast->m_declaration.get());
	typed_for->m_condition = convertAST(ast->m_condition.get());
	typed_for->m_action = convertAST(ast->m_action.get());
	typed_for->m_body = convertAST(ast->m_body.get());

	return typed_for;
}

Own<TypedAST> convertAST(AST::WhileStatement* ast) {
	auto typed_while = std::make_unique<WhileStatement>();

	typed_while->m_condition = convertAST(ast->m_condition.get());
	typed_while->m_body = convertAST(ast->m_body.get());

	return typed_while;
}

Own<TypedAST> convertAST(AST::AST* ast) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return convertAST(static_cast<AST::type*>(ast))

#define REJECT(type)                                                           \
	case ASTTag::type:                                                         \
		assert(0 && "use of " #type " is forbidden in convert")

	switch (ast->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(DictionaryLiteral);
		DISPATCH(FunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(RecordAccessExpression);
		REJECT(BinaryExpression);

		DISPATCH(Block);
		DISPATCH(ReturnStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);

		DISPATCH(DeclarationList);
		DISPATCH(Declaration);
	}
	std::cerr << "Error: AST type not handled in convertAST: "
	          << ast_string[(int)ast->type()] << std::endl;
	assert(0);

#undef REJECT
#undef DISPATCH
}

} // namespace TypedAST
