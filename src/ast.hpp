#pragma once

#include <memory>
#include <vector>
#include <string>

#include "token.hpp"
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
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

	ASTNumberLiteral() : AST{ ast_type::NumberLiteral } {}
};

struct ASTStringLiteral : public AST {
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

	ASTStringLiteral() : AST{ ast_type::StringLiteral } {}
};

struct ASTBooleanLiteral : public AST {
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

	ASTBooleanLiteral() : AST{ ast_type::BooleanLiteral } {}
};

struct ASTNullLiteral : public AST {

	ASTNullLiteral() : AST{ ast_type::NullLiteral } {}
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

	ASTFunctionLiteral() : AST{ ast_type::FunctionLiteral } {}
};


struct ASTDeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	ASTDeclarationList() : AST{ ast_type::DeclarationList } {}
};

struct ASTDeclaration : public AST {
	Token const* m_identifier_token;
	Token const* m_typename_token { nullptr };
	std::unique_ptr<AST> m_value;

	std::string const& identifier_text() const { return m_identifier_token->m_text; }
	std::string const& typename_text() const { return m_typename_token->m_text; }

	ASTDeclaration() : AST{ ast_type::Declaration } {}
};

struct ASTIdentifier : public AST {
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

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
	std::vector<std::unique_ptr<AST>> m_args;

	ASTCallExpression() : AST{ ast_type::CallExpression } {}
};

struct ASTIndexExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_index;

	ASTIndexExpression() : AST{ ast_type::IndexExpression } {}
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

struct ASTForStatement : public AST {
	std::unique_ptr<AST> m_declaration;
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_action;
	std::unique_ptr<AST> m_body;

	ASTForStatement() : AST{ ast_type::ForStatement } {}
};

void print (AST*, int);
