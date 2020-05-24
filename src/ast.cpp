#include "ast.hpp"

#include <iostream>

#include <cassert>

#include "value.hpp"

namespace {

constexpr char tabc = ' ';

}

void print(ASTDeclarationList* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ DeclarationList\n";
	for (auto& decl : ast->m_declarations)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTDeclaration* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Declaration\n"
		<< tab << "Name: " << ast->identifier_text() << '\n';

	if(ast->m_typename_token)
		std::cout << tab << "Type: " <<  ast->typename_text() << '\n';

	if (ast->m_value) {
		std::cout << tab << "Initializer:\n";
		print(ast->m_value.get(), d + 1);
	}
	std::cout << stab << "]\n";
}

void print(ASTNumberLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ Number " << ast->text() << " ]\n";
}

void print(ASTStringLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ StringLiteral " << ast->text() << "]\n";
}

void print(ASTBooleanLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ BooleanLiteral " << ast->text() << "]\n";
}

void print(ASTNullLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ NullLiteral ]\n";
}

void print(ASTObjectLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ObjectLiteral\n"
		<< tab << "Declarations:\n";
	for (auto& decl : ast->m_body)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTArrayLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ArrayLiteral\n"
		<< tab << "Elements:\n";
	for (auto& elem : ast->m_elements)
		print(elem.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTDictionaryLiteral* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DictionaryLiteral\n"
		<< tab << "Declarations:\n";
	for (auto& decl : ast->m_body)
		print(decl.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTIdentifier* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ Identifier " << ast->text() << " ]\n";
}

void print(ASTBlock* ast, int d) {
	std::string stab(d - 1, tabc);
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
	for(auto& arg : ast->m_args)
		print(arg.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTIndexExpression* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ IndexExpression\n"
	          << tab << "Callee:\n";
	print(ast->m_callee.get(), d + 1);
	std::cout << tab << "Index:\n";
	print(ast->m_index.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTReturnStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::cout << stab << "[ ReturnStatement\n";
	print(ast->m_value.get(), d+1);
	std::cout << stab << "]\n";
}

void print(ASTIfStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ IfStatement\n"
	          << tab << "Condition:\n";
	print(ast->m_condition.get(), d + 1);
	std::cout << tab << "Body:\n";
	print(ast->m_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(ASTForStatement* ast, int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ForStatement\n"
	          << tab << "Declaration:\n";
	print(ast->m_declaration.get(), d + 1);
	std::cout << tab << "Condition:\n";
	print(ast->m_condition.get(), d + 1);
	std::cout << tab << "Action:\n";
	print(ast->m_action.get(), d + 1);
	std::cout << tab << "Body:\n";
	print(ast->m_body.get(), d + 1);
	std::cout << stab << "]\n";
}

void print(AST* ast, int d) {
	// TODO missing argument list printer
	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return print(static_cast<ASTNumberLiteral*>(ast), d);
	case ast_type::StringLiteral:
		return print(static_cast<ASTStringLiteral*>(ast), d);
	case ast_type::BooleanLiteral:
		return print(static_cast<ASTBooleanLiteral*>(ast), d);
	case ast_type::NullLiteral:
		return print(static_cast<ASTNullLiteral*>(ast), d);
	case ast_type::ObjectLiteral:
		return print(static_cast<ASTObjectLiteral*>(ast), d);
	case ast_type::ArrayLiteral:
		return print(static_cast<ASTArrayLiteral*>(ast), d);
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
	case ast_type::IndexExpression:
		return print(static_cast<ASTIndexExpression*>(ast), d);
	case ast_type::Block:
		return print(static_cast<ASTBlock*>(ast), d);
	case ast_type::ReturnStatement:
		return print(static_cast<ASTReturnStatement*>(ast), d);
	case ast_type::IfStatement:
		return print(static_cast<ASTIfStatement*>(ast), d);
	case ast_type::ForStatement:
		return print(static_cast<ASTForStatement*>(ast), d);
	}

}
