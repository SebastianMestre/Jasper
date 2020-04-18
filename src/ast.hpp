#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "token_type.hpp"

struct AST {
	virtual void print(int d = 1) = 0;
	virtual ~AST() = default;
};

constexpr char tabc = ' ';

struct ASTDeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ DeclarationList\n";
		for (auto& ast : m_declarations)
			ast->print(d + 1);
		std::cout << stab << "]\n";
	}
};

struct ASTDeclaration : public AST {
	std::string m_identifier;
	std::string m_typename;
	std::unique_ptr<AST> m_value;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ Declaration\n"
		          << tab << "Name: " << m_identifier << '\n'
		          << tab << "Type: " << m_typename << '\n'
		          << tab << "Initializer:\n";
		m_value->print(d + 1);
		std::cout << stab << "]\n";
	}
};

struct ASTNumberLiteral : public AST {
	std::string m_text;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ Number\n"
		          << tab << "Value: " << m_text << "\n"
		          << stab << "]\n";
	}
};

struct ASTIdentifier : public AST {
	std::string m_text;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ Identifier\n"
		          << tab << "Name: " << m_text << '\n'
		          << stab << "]\n";
	}
};

struct ASTBinaryExpression : public AST {
	token_type m_op;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ BinaryExpression\n"
		          << tab << "Operator: " << token_type_string[int(m_op)] << '\n'
		          << tab << "Left Operand:\n";
		m_lhs->print(d+1);
		std::cout << tab << "Right Operand:\n";
		m_rhs->print(d+1);
		std::cout << stab << "]\n";
	}
};

struct ASTBlock : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ Block\n";
		for (auto& child : m_body)
			child->print(d + 1);
		std::cout << stab << "]\n";
	}
};

struct ASTFunction : public AST {
	std::unique_ptr<AST> m_body;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ Function\n";
		if (m_body)
			m_body->print(d + 1);
		std::cout << stab << "]\n";
	}
};
