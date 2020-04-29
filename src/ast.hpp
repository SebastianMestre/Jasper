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


struct ASTNumberLiteral : public AST {
	std::string m_text;

	void print(int d) override;
};

struct ASTStringLiteral : public AST {
	std::string m_text;

	void print(int d) override;
};

struct ASTObjectLiteral : public AST {
	std::unique_ptr<AST> m_body;
	void print(int d) override;
};

struct ASTDictionaryLiteral : public AST {
	std::unique_ptr<AST> m_body;
	void print(int d) override;
};

struct ASTFunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	void print(int d) override;
};


struct ASTDeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	void print(int d) override;
};

struct ASTDeclaration : public AST {
	std::string m_identifier;
	std::string m_typename;
	std::unique_ptr<AST> m_value;

	void print(int d) override;
};

struct ASTIdentifier : public AST {
	std::string m_text;

	void print(int d) override;
};

struct ASTBinaryExpression : public AST {
	token_type m_op;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	void print(int d) override;
};

struct ASTCallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_args;

	void print(int d) override;
};

struct ASTArgumentList : public AST {
	std::vector<std::unique_ptr<AST>> m_args;

	void print(int d) override;
};

struct ASTBlock : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	void print(int d) override;
};

struct ASTReturnStatement : public AST {
	std::unique_ptr<AST> m_value;

	void print(int d) override;
};
