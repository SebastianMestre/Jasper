#include "eval.hpp"

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"
#include "types.hpp"


Type::Value* eval(ASTDeclarationList* ast, Type::Environment& e) {
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		eval(static_cast<ASTDeclaration*>(decl.get()), e);
	}
	return nullptr;
};

Type::Value* eval(ASTDeclaration* ast, Type::Environment& e) {
	// TODO: type and mutable check -> return error
	if (!ast->m_value)
		e.m_scope->declare(ast->m_identifier, e.null());
	else
		e.m_scope->declare(ast->m_identifier, eval(ast->m_value.get(), e));

	return e.null();
};

Type::Value* eval(ASTNumberLiteral* ast, Type::Environment& e) {
	for (char a : ast->m_text)
		if (a == '.')
			return e.new_float(std::stof(ast->m_text));

	return e.new_integer(std::stoi(ast->m_text));
};

Type::Value* eval(ASTStringLiteral* ast, Type::Environment& e) {
	return e.new_string(ast->m_text);
};

Type::Value* eval(ASTObjectLiteral* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Creating an object)\n";
	return nullptr;
};

Type::Value* eval(ASTDictionaryLiteral* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Creating a dictionary)\n";
	return nullptr;
};

Type::Value* eval(ASTIdentifier* ast, Type::Environment& e) {
	return e.m_scope->access(ast->m_text);
};

Type::Value* eval(ASTBlock* ast, Type::Environment& e) {
	return e.null();
};

Type::Value* eval(ASTReturnStatement* ast, Type::Environment& e) {
	// TODO: implement
	std::cerr << "WARNING: not implemented action (return statement)\n";
	return e.null();
};

Type::Value* eval(ASTArgumentList* ast, Type::Environment& e) {
	// TODO: return as list?
	assert(0);
	return e.null();
};

Type::Value* eval(ASTCallExpression* ast, Type::Environment& e) {
	// TODO: fetch function definition and scope and run
	assert(0);
	return e.null();
};

Type::Value* eval(ASTBinaryExpression* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Evaluating binary expression)\n";
	return nullptr;
}

Type::Value* eval(ASTFunctionLiteral* ast, Type::Environment& e) {
	return e.new_function(ast, e.m_scope);
};

Type::Value* eval(AST* ast, Type::Environment& e) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return eval(static_cast<ASTNumberLiteral*>(ast), e);
	case ast_type::StringLiteral:
		return eval(static_cast<ASTStringLiteral*>(ast), e);
	case ast_type::ObjectLiteral:
		return eval(static_cast<ASTObjectLiteral*>(ast), e);
	case ast_type::DictionaryLiteral:
		return eval(static_cast<ASTDictionaryLiteral*>(ast), e);
	case ast_type::FunctionLiteral:
		return eval(static_cast<ASTFunctionLiteral*>(ast), e);
	case ast_type::DeclarationList:
		return eval(static_cast<ASTDeclarationList*>(ast), e);
	case ast_type::Declaration:
		return eval(static_cast<ASTDeclaration*>(ast), e);
	case ast_type::Identifier:
		return eval(static_cast<ASTIdentifier*>(ast), e);
	case ast_type::BinaryExpression:
		return eval(static_cast<ASTBinaryExpression*>(ast), e);
	case ast_type::CallExpression:
		return eval(static_cast<ASTCallExpression*>(ast), e);
	case ast_type::ArgumentList:
		return eval(static_cast<ASTArgumentList*>(ast), e);
	case ast_type::Block:
		return eval(static_cast<ASTBlock*>(ast), e);
	case ast_type::ReturnStatement:
		return eval(static_cast<ASTReturnStatement*>(ast), e);
	}

	return nullptr;
}
