#include "eval.hpp"

#include "ast.hpp"
#include "environment.hpp"
#include "runtime.hpp"
#include "types.hpp"


Type::Value* eval(ASTDeclarationList* ast, Type::Environment& e) {
	for(auto& decl : ast->m_declarations){
		assert(dynamic_cast<ASTDeclaration*>(decl.get()));
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

	// TODO: replace by switch on enum

	{
		auto* t = dynamic_cast<ASTNumberLiteral*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTStringLiteral*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTObjectLiteral*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTDictionaryLiteral*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTFunctionLiteral*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTDeclarationList*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTDeclaration*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTIdentifier*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTBinaryExpression*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTCallExpression*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTArgumentList*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTBlock*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTReturnStatement*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}
	{
		auto* t = dynamic_cast<ASTNumberLiteral*>(ast);
		if (t != nullptr) {
			return eval(t, e);
		}
	}

	return nullptr;
}
