#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "token_type.hpp"
#include "ast_type.hpp"

struct AST {
protected:
	ast_type m_type;

public:
	AST() = default;
	AST(ast_type type) : m_type{ type } {}

	ast_type type() const { return m_type; }

	virtual void print(int d = 1) = 0;
	virtual ~AST() = default;
};

constexpr char tabc = ' ';


struct ASTNumberLiteral : public AST {
	std::string m_text;

	ASTNumberLiteral() : AST{ ast_type::NumberLiteral } {}

	void print(int d) override;
};

struct ASTStringLiteral : public AST {
	std::string m_text;

	ASTStringLiteral() : AST{ ast_type::StringLiteral } {}

	void print(int d) override;
};

struct ASTObjectLiteral : public AST {
	std::unique_ptr<AST> m_body;

	ASTObjectLiteral() : AST{ ast_type::ObjectLiteral } {}

	void print(int d) override;
};

struct ASTDictionaryLiteral : public AST {
	std::unique_ptr<AST> m_body;

	ASTDictionaryLiteral() : AST{ ast_type::DictionaryLiteral } {}
	void print(int d) override;
};

struct ASTFunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	ASTFunctionLiteral() : AST{ ast_type::FunctionLiteral } {}

	void print(int d) override;
};


struct ASTDeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	ASTDeclarationList() : AST{ ast_type::DeclarationList } {}
	void print(int d) override;
};

struct ASTDeclaration : public AST {
	std::string m_identifier;
	std::string m_typename;
	std::unique_ptr<AST> m_value;

	ASTDeclaration() : AST{ ast_type::Declaration } {}
	void print(int d) override;
};

struct ASTIdentifier : public AST {
	std::string m_text;

	ASTIdentifier() : AST{ ast_type::Identifier } {}
	void print(int d) override;
};

struct ASTBinaryExpression : public AST {
	token_type m_op;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	ASTBinaryExpression() : AST{ ast_type::BinaryExpression } {}
	void print(int d) override;
};

struct ASTCallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_args;

	ASTCallExpression() : AST{ ast_type::CallExpression } {}
	void print(int d) override;
};

struct ASTArgumentList : public AST {
	std::vector<std::unique_ptr<AST>> m_args;

	ASTArgumentList() : AST{ ast_type::ArgumentList } {}
	void print(int d) override;
};

struct ASTBlock : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ASTBlock() : AST{ ast_type::Block } {}
	void print(int d) override;
};

struct ASTReturnStatement : public AST {
	std::unique_ptr<AST> m_value;

	ASTReturnStatement() : AST{ ast_type::ReturnStatement } {}
	void print(int d) override;
};
