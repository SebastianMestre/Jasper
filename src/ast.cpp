#include <cassert>

#include "ast.hpp"
#include "runtime.hpp"
#include "garbage_collector.hpp"

void ASTDeclarationList::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DeclarationList\n";
	for (auto& ast : m_declarations)
		ast->print(d + 1);
	std::cout << stab << "]\n";
}

void ASTDeclaration::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Declaration\n"
		<< tab << "Name: " << m_identifier << '\n'
		<< tab << "Type: " << m_typename << '\n';
	if (m_value) {
		std::cout << tab << "Initializer:\n";
		m_value->print(d + 1);
	}
	std::cout << stab << "]\n";
}

void ASTNumberLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Number " << m_text << " ]\n";
}

void ASTStringLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ StringLiteral\n"
		<< tab << "Value: " << m_text << "\n"
		<< stab << "]\n";
}

void ASTObjectLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ObjectLiteral\n"
		<< tab << "Declarations:\n";
	m_body->print(d+1);
	std::cout << stab << "]\n";
}

void ASTDictionaryLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ DictionaryLiteral\n"
		<< tab << "Declarations:\n";
	m_body->print(d+1);
	std::cout << stab << "]\n";
}

void ASTIdentifier::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Identifier\n"
		<< tab << "Name: " << m_text << '\n'
		<< stab << "]\n";
}

void ASTBlock::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Block\n";
	for (auto& child : m_body)
		child->print(d + 1);
	std::cout << stab << "]\n";
}

void ASTFunctionLiteral::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ Function\n"
		<< tab << "Arguments:\n";
	for(auto& arg : m_args){
		arg->print(d+1);
	}
	std::cout << tab << "Body:\n";
	if (m_body)
		m_body->print(d + 1);
	std::cout << stab << "]\n";
}

void ASTBinaryExpression::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ BinaryExpression\n"
	          << tab << "Operator: " << token_type_string[int(m_op)] << '\n'
	          << tab << "Left Operand:\n";
	m_lhs->print(d + 1);
	std::cout << tab << "Right Operand:\n";
	m_rhs->print(d + 1);
	std::cout << stab << "]\n";
}

void ASTCallExpression::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ CallExpression\n"
	          << tab << "Callee:\n";
	m_callee->print(d + 1);
	std::cout << tab << "Args:\n";
	m_args->print(d + 1);
	std::cout << stab << "]\n";
}

void ASTArgumentList::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ArgumentList\n";
	for(auto& argument : m_args){
		argument->print(d+1);
	}
	std::cout << stab << "]\n";
}

void ASTReturnStatement::print(int d) {
	std::string stab(d - 1, tabc);
	std::string tab(d, tabc);
	std::cout << stab << "[ ReturnStatement\n";
	m_value->print(d+1);
	std::cout << stab << "]\n";
}

