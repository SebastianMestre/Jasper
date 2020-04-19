#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "types.hpp"
#include "runtime.hpp"

constexpr char tabc = '\t';

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

	// TODO: implement with GC and with actual values that make sense
	virtual Type::Value* run(Type::Scope &s) override {
		return new Type::Null();
	};
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
	
	// TODO: implement with GC and with actual values that make sense
	virtual Type::Value* run(Type::Scope &s) override {
		return new Type::Null();
	};
};

struct ASTNumberLiteral : public AST {
	std::string m_text;

	void print(int d) override {
		std::string stab(d - 1, tabc);
		std::string tab(d, tabc);
		std::cout << stab << "[ Number\n"
		          << tab << "Type::Value: " << m_text << "\n"
		          << stab << "]\n";
	}
	
	// TODO: implement with GC and with actual values that make sense
	virtual Type::Value* run(Type::Scope &s) override {
		return new Type::Null();
	};
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
	
	// TODO: implement with GC and with actual values that make sense
	virtual Type::Value* run(Type::Scope &s) override {
		return new Type::Null();
	};
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
	
	// TODO: implement with GC and with actual values that make sense
	virtual Type::Value* run(Type::Scope &s) override {
		return new Type::Null();
	};
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
	
	// TODO: implement with GC and with actual values that make sense
	virtual Type::Value* run(Type::Scope &s) override {
		return new Type::Null();
	};
};
