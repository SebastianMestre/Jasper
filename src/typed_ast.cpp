#include <cassert>
#include <iostream>

#include "ast.hpp"
#include "typed_ast.hpp"

namespace TypedAST {

std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST::AST>& ast) {
	return std::unique_ptr<TypedAST>(convertAST(ast.get()));
}

TypedAST* convertAST(AST::IntegerLiteral* ast) {
	auto typed_integer = new IntegerLiteral;
	typed_integer->m_token = ast->m_token;
	return typed_integer;
}

TypedAST* convertAST(AST::NumberLiteral* ast) {
	auto typed_number = new NumberLiteral;
	typed_number->m_token = ast->m_token;
	return typed_number;
}

TypedAST* convertAST(AST::StringLiteral* ast) {
	auto typed_string = new StringLiteral;
	typed_string->m_token = ast->m_token;
	return typed_string;
}

TypedAST* convertAST(AST::BooleanLiteral* ast) {
	auto typed_boolean = new BooleanLiteral;
	typed_boolean->m_token = ast->m_token;
	return typed_boolean;
}

TypedAST* convertAST(AST::NullLiteral* ast) {
	return new NullLiteral;
}

TypedAST* convertAST(AST::ObjectLiteral* ast) {
	auto typed_object = new ObjectLiteral;
	// al tipo de los objetos se les debe sumar una identificacion
	// de clase

	for (auto& element : ast->m_body) {
		typed_object->m_body.push_back(get_unique(element));
	}

	return typed_object;
}

TypedAST* convertAST(AST::ArrayLiteral* ast) {
	auto typed_array = new ArrayLiteral;

	for (auto& element : ast->m_elements) {
		typed_array->m_elements.push_back(get_unique(element));
	}

	return typed_array;
}

TypedAST* convertAST(AST::DictionaryLiteral* ast) {
	auto typed_dict = new DictionaryLiteral;

	for (auto& element : ast->m_body) {
		typed_dict->m_body.push_back(get_unique(element));
	}

	return typed_dict;
}

TypedAST* convertAST(AST::FunctionLiteral* ast) {
	auto typed_function = new FunctionLiteral;

	for (auto& arg : ast->m_args) {
		assert(arg->type() == ASTType::Declaration);
		auto* decl = static_cast<AST::Declaration*>(arg.get());

		typed_function->m_args.push_back({decl->m_identifier_token});
	}

	typed_function->m_body = get_unique(ast->m_body);

	return typed_function;
}

TypedAST* convertAST(AST::DeclarationList* ast) {
	auto typed_declist = new DeclarationList;

	for (auto& declaration : ast->m_declarations) {
		typed_declist->m_declarations.push_back(get_unique(declaration));
	}

	return typed_declist;
}

TypedAST* convertAST(AST::Declaration* ast) {
	auto typed_dec = new Declaration;

	typed_dec->m_identifier_token = ast->m_identifier_token;
	// TODO: handle type hint
	if (ast->m_value)
		typed_dec->m_value = get_unique(ast->m_value);

	return typed_dec;
}

TypedAST* convertAST(AST::Identifier* ast) {
	auto typed_id = new Identifier;
	typed_id->m_token = ast->m_token;
	return typed_id;
}

TypedAST* convertAST(AST::CallExpression* ast) {
	auto typed_ce = new CallExpression;

	for (auto& arg : ast->m_args) {
		typed_ce->m_args.push_back(get_unique(arg));
	}

	typed_ce->m_callee = get_unique(ast->m_callee);

	return typed_ce;
}

TypedAST* convertAST(AST::IndexExpression* ast) {
	auto typed_index = new IndexExpression;

	typed_index->m_callee = get_unique(ast->m_callee);
	typed_index->m_index = get_unique(ast->m_index);

	return typed_index;
}

TypedAST* convertAST(AST::TernaryExpression* ast) {
	auto typed_ternary = new TernaryExpression;

	typed_ternary->m_condition = get_unique(ast->m_condition);
	typed_ternary->m_then_expr = get_unique(ast->m_then_expr);
	typed_ternary->m_else_expr = get_unique(ast->m_else_expr);

	return typed_ternary;
}

TypedAST* convertAST(AST::Block* ast) {
	auto typed_block = new Block;

	for (auto& element : ast->m_body) {
		typed_block->m_body.push_back(get_unique(element));
	}

	return typed_block;
}

TypedAST* convertAST(AST::ReturnStatement* ast) {
	auto typed_rs = new ReturnStatement;

	typed_rs->m_value = get_unique(ast->m_value);

	return typed_rs;
}

TypedAST* convertAST(AST::IfElseStatement* ast) {
	auto typed_if_else = new IfElseStatement;

	typed_if_else->m_condition = get_unique(ast->m_condition);
	typed_if_else->m_body = get_unique(ast->m_body);

	if (ast->m_else_body)
		typed_if_else->m_else_body = get_unique(ast->m_else_body);

	return typed_if_else;
}

TypedAST* convertAST(AST::ForStatement* ast) {
	auto typed_for = new ForStatement;

	typed_for->m_declaration = get_unique(ast->m_declaration);
	typed_for->m_condition = get_unique(ast->m_condition);
	typed_for->m_action = get_unique(ast->m_action);
	typed_for->m_body = get_unique(ast->m_body);

	return typed_for;
}

TypedAST* convertAST(AST::WhileStatement* ast) {
	auto typed_while = new WhileStatement;

	typed_while->m_condition = get_unique(ast->m_condition);
	typed_while->m_body = get_unique(ast->m_body);

	return typed_while;
}

TypedAST* convertAST(AST::AST* ast) {
	switch (ast->type()) {
	case ASTType::NumberLiteral:
		return convertAST(static_cast<AST::NumberLiteral*>(ast));
	case ASTType::IntegerLiteral:
		return convertAST(static_cast<AST::IntegerLiteral*>(ast));
	case ASTType::StringLiteral:
		return convertAST(static_cast<AST::StringLiteral*>(ast));
	case ASTType::BooleanLiteral:
		return convertAST(static_cast<AST::BooleanLiteral*>(ast));
	case ASTType::NullLiteral:
		return convertAST(static_cast<AST::NullLiteral*>(ast));
	case ASTType::ObjectLiteral:
		return convertAST(static_cast<AST::ObjectLiteral*>(ast));
	case ASTType::ArrayLiteral:
		return convertAST(static_cast<AST::ArrayLiteral*>(ast));
	case ASTType::DictionaryLiteral:
		return convertAST(static_cast<AST::DictionaryLiteral*>(ast));
	case ASTType::FunctionLiteral:
		return convertAST(static_cast<AST::FunctionLiteral*>(ast));
	case ASTType::DeclarationList:
		return convertAST(static_cast<AST::DeclarationList*>(ast));
	case ASTType::Declaration:
		return convertAST(static_cast<AST::Declaration*>(ast));
	case ASTType::Identifier:
		return convertAST(static_cast<AST::Identifier*>(ast));
	case ASTType::CallExpression:
		return convertAST(static_cast<AST::CallExpression*>(ast));
	case ASTType::IndexExpression:
		return convertAST(static_cast<AST::IndexExpression*>(ast));
	case ASTType::TernaryExpression:
		return convertAST(static_cast<AST::TernaryExpression*>(ast));
	case ASTType::Block:
		return convertAST(static_cast<AST::Block*>(ast));
	case ASTType::ReturnStatement:
		return convertAST(static_cast<AST::ReturnStatement*>(ast));
	case ASTType::IfElseStatement:
		return convertAST(static_cast<AST::IfElseStatement*>(ast));
	case ASTType::ForStatement:
		return convertAST(static_cast<AST::ForStatement*>(ast));
	case ASTType::WhileStatement:
		return convertAST(static_cast<AST::WhileStatement*>(ast));
	case ASTType::BinaryExpression:
		assert(0);
	}
	std::cerr << "Error: AST type not handled in convertAST: "
	          << ast_type_string[(int)ast->type()] << std::endl;
	assert(0);
}

} // namespace TypedAST
