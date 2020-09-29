#include <cassert>
#include <iostream>

#include "ast.hpp"
#include "typed_ast.hpp"
#include "typedefs.hpp"

namespace TypedAST {

Own<TypedAST> convert_ast(AST::IntegerLiteral* ast) {
	auto typed_integer = std::make_unique<IntegerLiteral>();
	typed_integer->m_token = ast->m_token;
	return typed_integer;
}

Own<TypedAST> convert_ast(AST::NumberLiteral* ast) {
	auto typed_number = std::make_unique<NumberLiteral>();
	typed_number->m_token = ast->m_token;
	return typed_number;
}

Own<TypedAST> convert_ast(AST::StringLiteral* ast) {
	auto typed_string = std::make_unique<StringLiteral>();
	typed_string->m_token = ast->m_token;
	return typed_string;
}

Own<TypedAST> convert_ast(AST::BooleanLiteral* ast) {
	auto typed_boolean = std::make_unique<BooleanLiteral>();
	typed_boolean->m_token = ast->m_token;
	return typed_boolean;
}

Own<TypedAST> convert_ast(AST::NullLiteral* ast) {
	return std::make_unique<NullLiteral>();
}

Own<TypedAST> convert_ast(AST::ArrayLiteral* ast) {
	auto typed_array = std::make_unique<ArrayLiteral>();

	for (auto& element : ast->m_elements) {
		typed_array->m_elements.push_back(convert_ast(element.get()));
	}

	return typed_array;
}

Own<TypedAST> convert_ast(AST::DictionaryLiteral* ast) {
	auto typed_dict = std::make_unique<DictionaryLiteral>();

	for (auto& element : ast->m_body) {
		typed_dict->m_body.push_back(convert_ast(element.get()));
	}

	return typed_dict;
}

Own<TypedAST> convert_ast(AST::FunctionLiteral* ast) {
	auto typed_function = std::make_unique<FunctionLiteral>();

	for (auto& arg : ast->m_args) {
		assert(arg->type() == ASTTag::Declaration);
		auto* decl = static_cast<AST::Declaration*>(arg.get());

		Declaration typed_decl;
		typed_decl.m_identifier_token = decl->m_identifier_token;
		typed_decl.m_surrounding_function = typed_function.get();
		typed_function->m_args.push_back(std::move(typed_decl));
	}

	typed_function->m_body = convert_ast(ast->m_body.get());

	return typed_function;
}

Own<Declaration> convert_ast(AST::Declaration* ast) {
	auto typed_dec = std::make_unique<Declaration>();

	typed_dec->m_identifier_token = ast->m_identifier_token;
	// TODO: handle type hint
	if (ast->m_value)
		typed_dec->m_value = convert_ast(ast->m_value.get());

	return typed_dec;
}

Own<TypedAST> convert_ast(AST::DeclarationList* ast) {
	auto typed_declist = std::make_unique<DeclarationList>();

	for (auto& declaration : ast->m_declarations) {
		typed_declist->m_declarations.push_back(convert_ast(declaration.get()));
	}

	return typed_declist;
}

Own<TypedAST> convert_ast(AST::Identifier* ast) {
	auto typed_id = std::make_unique<Identifier>();
	typed_id->m_token = ast->m_token;
	return typed_id;
}

Own<TypedAST> convert_ast(AST::CallExpression* ast) {
	auto typed_ce = std::make_unique<CallExpression>();

	for (auto& arg : ast->m_args) {
		typed_ce->m_args.push_back(convert_ast(arg.get()));
	}

	typed_ce->m_callee = convert_ast(ast->m_callee.get());

	return typed_ce;
}

Own<TypedAST> convert_ast(AST::IndexExpression* ast) {
	auto typed_index = std::make_unique<IndexExpression>();

	typed_index->m_callee = convert_ast(ast->m_callee.get());
	typed_index->m_index = convert_ast(ast->m_index.get());

	return typed_index;
}

Own<TypedAST> convert_ast(AST::TernaryExpression* ast) {
	auto typed_ternary = std::make_unique<TernaryExpression>();

	typed_ternary->m_condition = convert_ast(ast->m_condition.get());
	typed_ternary->m_then_expr = convert_ast(ast->m_then_expr.get());
	typed_ternary->m_else_expr = convert_ast(ast->m_else_expr.get());

	return typed_ternary;
}

Own<TypedAST> convert_ast(AST::RecordAccessExpression* ast) {
	auto typed_ast = std::make_unique<RecordAccessExpression>();

	typed_ast->m_member = ast->m_member;
	typed_ast->m_record = convert_ast(ast->m_record.get());

	return typed_ast;
}

Own<TypedAST> convert_ast(AST::Block* ast) {
	auto typed_block = std::make_unique<Block>();

	for (auto& element : ast->m_body) {
		typed_block->m_body.push_back(convert_ast(element.get()));
	}

	return typed_block;
}

Own<TypedAST> convert_ast(AST::ReturnStatement* ast) {
	auto typed_rs = std::make_unique<ReturnStatement>();

	typed_rs->m_value = convert_ast(ast->m_value.get());

	return typed_rs;
}

Own<TypedAST> convert_ast(AST::IfElseStatement* ast) {
	auto typed_if_else = std::make_unique<IfElseStatement>();

	typed_if_else->m_condition = convert_ast(ast->m_condition.get());
	typed_if_else->m_body = convert_ast(ast->m_body.get());

	if (ast->m_else_body)
		typed_if_else->m_else_body = convert_ast(ast->m_else_body.get());

	return typed_if_else;
}

Own<TypedAST> convert_ast(AST::ForStatement* ast) {
	auto typed_for = std::make_unique<ForStatement>();

	typed_for->m_declaration = convert_ast(ast->m_declaration.get());
	typed_for->m_condition = convert_ast(ast->m_condition.get());
	typed_for->m_action = convert_ast(ast->m_action.get());
	typed_for->m_body = convert_ast(ast->m_body.get());

	return typed_for;
}

Own<TypedAST> convert_ast(AST::WhileStatement* ast) {
	auto typed_while = std::make_unique<WhileStatement>();

	typed_while->m_condition = convert_ast(ast->m_condition.get());
	typed_while->m_body = convert_ast(ast->m_body.get());

	return typed_while;
}

Own<TypedAST> convert_ast(AST::StructExpression* ast) {
	auto typed_ast = std::make_unique<StructExpression>();

	for (auto& field : ast->m_fields){
		auto ptr = convert_ast(&field);
		typed_ast->m_fields.push_back(std::move(*static_cast<Identifier*>(ptr.get())));
	}

	for (auto& type : ast->m_types){
		typed_ast->m_types.push_back(convert_ast(type.get()));
	}

	return typed_ast;
};

Own<TypedAST> convert_ast(AST::TypeTerm* ast) {
	auto typed_ast = std::make_unique<TypeTerm>();

	typed_ast->m_callee = convert_ast(ast->m_callee.get());
	for (auto& arg : ast->m_args){
		typed_ast->m_args.push_back(convert_ast(arg.get()));
	}

	return typed_ast;
}

Own<TypedAST> convert_ast(AST::AST* ast) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return convert_ast(static_cast<AST::type*>(ast))

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

		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}
	std::cerr << "Error: AST type not handled in convert_ast: "
	          << ast_string[(int)ast->type()] << std::endl;
	assert(0);

#undef REJECT
#undef DISPATCH
}

} // namespace TypedAST
