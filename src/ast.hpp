#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "types.hpp"
#include "token_type.hpp"

struct AST {
	virtual void print(int d = 1) = 0;
	virtual Type::Value* eval(Type::Environment &e) = 0;
	virtual ~AST() = default;
};

constexpr char tabc = ' ';


struct ASTNumberLiteral : public AST {
	std::string m_text;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTStringLiteral : public AST {
	std::string m_text;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTObjectLiteral : public AST {
	std::unique_ptr<AST> m_body;
	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTDictionaryLiteral : public AST {
	std::unique_ptr<AST> m_body;
	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTFunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};


struct ASTDeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTDeclaration : public AST {
	std::string m_identifier;
	std::string m_typename;
	std::unique_ptr<AST> m_value;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTIdentifier : public AST {
	std::string m_text;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTBinaryExpression : public AST {
	token_type m_op;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTCallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_args;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTArgumentList : public AST {
	std::vector<std::unique_ptr<AST>> m_args;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

struct ASTBlock : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	void print(int d) override;
	Type::Value* eval(Type::Environment &e) override;
};

