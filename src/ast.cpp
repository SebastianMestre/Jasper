#include <cassert>

#include "ast.hpp"
#include "value.hpp"
#include "garbage_collector.hpp"

void print(ASTDeclarationList* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DeclarationList\n";
	for (auto& decl : ast->m_declarations)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTDeclaration* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Declaration\n"
		<< tab << "Name: " << ast->m_identifier << '\n'
		<< tab << "Type: " << ast->m_typename << '\n';
	if (ast->m_value) {
		std::cout << tab << "Initializer:\n";
		print(ast->m_value.get(), d + 1);
	}
	std::cout << stab << "]\n";
}

void print(ASTNumberLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Number " << ast->m_text << " ]\n";
}

void print(ASTStringLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ StringLiteral\n"
		<< tab << "Value: " << ast->m_text << "\n"
		<< stab << "]\n";
}

void print(ASTObjectLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ObjectLiteral\n"
		<< tab << "Declarations:\n";
	print(ast->m_body.get(), d+1);
	std::cout << stab << "]\n";
}

void print(ASTDictionaryLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DictionaryLiteral\n"
		<< tab << "Declarations:\n";
	print(ast->m_body.get(), d+1);
	std::cout << stab << "]\n";
}

void print(ASTIdentifier* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Identifier\n"
		<< tab << "Name: " << ast->m_text << '\n'
		<< stab << "]\n";
}

void print(ASTBlock* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Block\n";
	for (auto& child : ast->m_body)
		print(child.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTFunctionLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Function\n"
		<< tab << "Arguments:\n";
	for(auto& arg : ast->m_args){
		print(arg.get(), d+1);
	}
	std::cout << tab << "Body:\n";
	if (ast->m_body)
		print(ast->m_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTBinaryExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ BinaryExpression\n"
	          << tab << "Operator: " << token_type_string[int(ast->m_op)] << '\n'
	          << tab << "Left Operand:\n";
	print(ast->m_lhs.get(), d + 1);
	std::cout << tab << "Right Operand:\n";
	print(ast->m_rhs.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTCallExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ CallExpression\n"
	          << tab << "Callee:\n";
	print(ast->m_callee.get(), d + 1);
	std::cout << tab << "Args:\n";
	print(ast->m_args.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTArgumentList* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ArgumentList\n";
	for(auto& argument : ast->m_args){
		print(argument.get(), d+1);
	}
	std::cout << stab << "]\n";
}

void print(ASTReturnStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ReturnStatement\n";
	print(ast->m_value.get(), d+1);
	std::cout << stab << "]\n";
}

void print(AST* ast, int d) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return print(static_cast<ASTNumberLiteral*>(ast), d);
	case ast_type::StringLiteral:
		return print(static_cast<ASTStringLiteral*>(ast), d);
	case ast_type::ObjectLiteral:
		return print(static_cast<ASTObjectLiteral*>(ast), d);
	case ast_type::DictionaryLiteral:
		return print(static_cast<ASTDictionaryLiteral*>(ast), d);
	case ast_type::FunctionLiteral:
		return print(static_cast<ASTFunctionLiteral*>(ast), d);
	case ast_type::DeclarationList:
		return print(static_cast<ASTDeclarationList*>(ast), d);
	case ast_type::Declaration:
		return print(static_cast<ASTDeclaration*>(ast), d);
	case ast_type::Identifier:
		return print(static_cast<ASTIdentifier*>(ast), d);
	case ast_type::BinaryExpression:
		return print(static_cast<ASTBinaryExpression*>(ast), d);
	case ast_type::CallExpression:
		return print(static_cast<ASTCallExpression*>(ast), d);
	case ast_type::ArgumentList:
		return print(static_cast<ASTArgumentList*>(ast), d);
	case ast_type::Block:
		return print(static_cast<ASTBlock*>(ast), d);
	case ast_type::ReturnStatement:
		return print(static_cast<ASTReturnStatement*>(ast), d);
	}

}
