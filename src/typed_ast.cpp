#include <cassert>
#include <iostream>

#include "ast.hpp"
#include "typed_ast.hpp"
#include "typed_ast_allocator.hpp"
#include "utils/typedefs.hpp"

namespace TypedAST {

TypedAST* convert_ast(AST::IntegerLiteral* ast, Allocator& alloc) {
	auto typed_integer = alloc.make<IntegerLiteral>();
	typed_integer->m_value = std::stoi(ast->text());
	typed_integer->m_token = ast->m_token;
	return typed_integer;
}

TypedAST* convert_ast(AST::NumberLiteral* ast, Allocator& alloc) {
	auto typed_number = alloc.make<NumberLiteral>();
	typed_number->m_value = std::stof(ast->text());
	typed_number->m_token = ast->m_token;
	return typed_number;
}

TypedAST* convert_ast(AST::StringLiteral* ast, Allocator& alloc) {
	auto typed_string = alloc.make<StringLiteral>();
	typed_string->m_token = ast->m_token;
	return typed_string;
}

TypedAST* convert_ast(AST::BooleanLiteral* ast, Allocator& alloc) {
	auto typed_boolean = alloc.make<BooleanLiteral>();
	typed_boolean->m_token = ast->m_token;
	return typed_boolean;
}

TypedAST* convert_ast(AST::NullLiteral* ast, Allocator& alloc) {
	return alloc.make<NullLiteral>();
}

TypedAST* convert_ast(AST::ArrayLiteral* ast, Allocator& alloc) {
	auto typed_array = alloc.make<ArrayLiteral>();

	for (auto element : ast->m_elements) {
		typed_array->m_elements.push_back(convert_ast(element, alloc));
	}

	return typed_array;
}

TypedAST* convert_ast(AST::DictionaryLiteral* ast, Allocator& alloc) {
	auto typed_dict = alloc.make<DictionaryLiteral>();

	for (auto& element : ast->m_body) {
		auto decl = static_cast<Declaration*>(convert_ast(&element, alloc));
		typed_dict->m_body.push_back(std::move(*decl));
	}

	return typed_dict;
}

TypedAST* convert_ast(AST::FunctionLiteral* ast, Allocator& alloc) {
	auto typed_function = alloc.make<FunctionLiteral>();

	for (auto& arg : ast->m_args) {
		Declaration typed_decl;

		typed_decl.m_identifier_token = arg.m_identifier_token;
		typed_decl.m_surrounding_function = typed_function;

		typed_function->m_args.push_back(std::move(typed_decl));
	}

	typed_function->m_body = convert_ast(ast->m_body, alloc);

	return typed_function;
}

TypedAST* convert_ast(AST::Declaration* ast, Allocator& alloc) {
	auto typed_dec = alloc.make<Declaration>();

	typed_dec->m_identifier_token = ast->m_identifier_token;
	// TODO: handle type hint
	if (ast->m_value)
		typed_dec->m_value = convert_ast(ast->m_value, alloc);

	return typed_dec;
}

TypedAST* convert_ast(AST::DeclarationList* ast, Allocator& alloc) {
	auto typed_declist = alloc.make<DeclarationList>();

	for (auto& declaration : ast->m_declarations) {
		auto decl = static_cast<Declaration*>(convert_ast(&declaration, alloc));
		typed_declist->m_declarations.push_back(std::move(*decl));
	}

	return typed_declist;
}

TypedAST* convert_ast(AST::Identifier* ast, Allocator& alloc) {
	auto typed_id = alloc.make<Identifier>();
	typed_id->m_token = ast->m_token;
	return typed_id;
}

TypedAST* convert_ast(AST::CallExpression* ast, Allocator& alloc) {
	auto typed_ce = alloc.make<CallExpression>();

	for (auto arg : ast->m_args) {
		typed_ce->m_args.push_back(convert_ast(arg, alloc));
	}

	typed_ce->m_callee = convert_ast(ast->m_callee, alloc);

	return typed_ce;
}

TypedAST* convert_ast(AST::IndexExpression* ast, Allocator& alloc) {
	auto typed_index = alloc.make<IndexExpression>();

	typed_index->m_callee = convert_ast(ast->m_callee, alloc);
	typed_index->m_index = convert_ast(ast->m_index, alloc);

	return typed_index;
}

TypedAST* convert_ast(AST::TernaryExpression* ast, Allocator& alloc) {
	auto typed_ternary = alloc.make<TernaryExpression>();

	typed_ternary->m_condition = convert_ast(ast->m_condition, alloc);
	typed_ternary->m_then_expr = convert_ast(ast->m_then_expr, alloc);
	typed_ternary->m_else_expr = convert_ast(ast->m_else_expr, alloc);

	return typed_ternary;
}

TypedAST* convert_ast(AST::RecordAccessExpression* ast, Allocator& alloc) {
	auto typed_ast = alloc.make<RecordAccessExpression>();

	typed_ast->m_member = ast->m_member;
	typed_ast->m_record = convert_ast(ast->m_record, alloc);

	return typed_ast;
}

TypedAST* convert_ast(AST::Block* ast, Allocator& alloc) {
	auto typed_block = alloc.make<Block>();

	for (auto element : ast->m_body) {
		typed_block->m_body.push_back(convert_ast(element, alloc));
	}

	return typed_block;
}

TypedAST* convert_ast(AST::ReturnStatement* ast, Allocator& alloc) {
	auto typed_rs = alloc.make<ReturnStatement>();

	typed_rs->m_value = convert_ast(ast->m_value, alloc);

	return typed_rs;
}

TypedAST* convert_ast(AST::IfElseStatement* ast, Allocator& alloc) {
	auto typed_if_else = alloc.make<IfElseStatement>();

	typed_if_else->m_condition = convert_ast(ast->m_condition, alloc);
	typed_if_else->m_body = convert_ast(ast->m_body, alloc);

	if (ast->m_else_body)
		typed_if_else->m_else_body = convert_ast(ast->m_else_body, alloc);

	return typed_if_else;
}

TypedAST* convert_ast(AST::ForStatement* ast, Allocator& alloc) {
	auto typed_for = alloc.make<ForStatement>();

	auto decl = static_cast<Declaration*>(convert_ast(&ast->m_declaration, alloc));
	typed_for->m_declaration = std::move(*decl);
	typed_for->m_condition = convert_ast(ast->m_condition, alloc);
	typed_for->m_action = convert_ast(ast->m_action, alloc);
	typed_for->m_body = convert_ast(ast->m_body, alloc);

	return typed_for;
}

TypedAST* convert_ast(AST::WhileStatement* ast, Allocator& alloc) {
	auto typed_while = alloc.make<WhileStatement>();

	typed_while->m_condition = convert_ast(ast->m_condition, alloc);
	typed_while->m_body = convert_ast(ast->m_body, alloc);

	return typed_while;
}

TypedAST* convert_ast(AST::StructExpression* ast, Allocator& alloc) {
	auto typed_ast = alloc.make<StructExpression>();

	for (auto& field : ast->m_fields){
		auto typed_field = static_cast<Identifier*>(convert_ast(&field, alloc));
		typed_ast->m_fields.push_back(std::move(*typed_field));
	}

	for (auto type : ast->m_types){
		typed_ast->m_types.push_back(convert_ast(type, alloc));
	}

	return typed_ast;
};

TypedAST* convert_ast(AST::TypeTerm* ast, Allocator& alloc) {
	auto typed_ast = alloc.make<TypeTerm>();

	typed_ast->m_callee = convert_ast(ast->m_callee, alloc);
	for (auto arg : ast->m_args){
		typed_ast->m_args.push_back(convert_ast(arg, alloc));
	}

	return typed_ast;
}

TypedAST* convert_ast(AST::AST* ast, Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return convert_ast(static_cast<AST::type*>(ast), alloc)

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
