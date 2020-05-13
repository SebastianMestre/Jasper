#pragma once

#include <memory>
#include <vector>
#include <string>

#include "token_type.hpp"
#include "ast_type.hpp"

struct AST {
protected:
	ast_type m_type;

public:
	AST() = default;
	AST(ast_type type) : m_type{ type } {}

	ast_type type() const { return m_type; }
	virtual ~AST() = default;
};


struct ASTNumberLiteral : public AST {
	std::string m_text;

	ASTNumberLiteral() : AST{ ast_type::NumberLiteral } {}
};

struct ASTStringLiteral : public AST {
	std::string m_text;

	ASTStringLiteral() : AST{ ast_type::StringLiteral } {}
};

struct ASTObjectLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ASTObjectLiteral() : AST{ ast_type::ObjectLiteral } {}
};

struct ASTArrayLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_elements;

	ASTArrayLiteral() : AST{ ast_type::ArrayLiteral } {}
};

struct ASTDictionaryLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ASTDictionaryLiteral() : AST{ ast_type::DictionaryLiteral } {}
};

struct ASTFunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;
	std::vector<std::string> m_captures;

	ASTFunctionLiteral() : AST{ ast_type::FunctionLiteral } {}
};


struct ASTDeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	ASTDeclarationList() : AST{ ast_type::DeclarationList } {}
};

struct ASTDeclaration : public AST {
	std::string m_identifier;
	std::string m_typename;
	std::unique_ptr<AST> m_value;

	ASTDeclaration() : AST{ ast_type::Declaration } {}
};

struct ASTIdentifier : public AST {
	std::string m_text;

	ASTIdentifier() : AST{ ast_type::Identifier } {}
};

struct ASTBinaryExpression : public AST {
	token_type m_op;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	ASTBinaryExpression() : AST{ ast_type::BinaryExpression } {}
};

struct ASTCallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_args;

	ASTCallExpression() : AST{ ast_type::CallExpression } {}
};

struct ASTArgumentList : public AST {
	std::vector<std::unique_ptr<AST>> m_args;

	ASTArgumentList() : AST{ ast_type::ArgumentList } {}
};

struct ASTBlock : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ASTBlock() : AST{ ast_type::Block } {}
};

struct ASTReturnStatement : public AST {
	std::unique_ptr<AST> m_value;

	ASTReturnStatement() : AST{ ast_type::ReturnStatement } {}
};

struct ASTIfStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;

	ASTIfStatement() : AST{ ast_type::IfStatement } {}
};

void print (AST*, int);
