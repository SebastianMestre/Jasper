#pragma once

#include <memory>
#include <vector>

#include "ast_type.hpp"
#include "token.hpp"
#include "token_type.hpp"
#include "value_type.hpp"
#include "typed_ast_type.hpp"
#include "hindleymilner.hpp"

struct AST;

struct TypedAST {
protected:
	ast_type m_type;

public:
    HindleyMilner::Poly m_vtype;
	TypedAST(ast_type type) : m_type{ type } {}
    TypedAST(ast_type type, ast_vtype vtype) : m_type {type} {}

	ast_type type() const { return m_type; }
	virtual ~TypedAST() = default;
};

TypedAST* convertAST(AST*);
void typeAST(TypedAST*);
bool valid_vtype(TypedAST*);
std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST>&);

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct TypedASTNumberLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

	TypedASTNumberLiteral() : 
        TypedAST{ ast_type::NumberLiteral, ast_vtype::Integer } {}
};

struct TypedASTStringLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

	TypedASTStringLiteral() : 
        TypedAST{ ast_type::StringLiteral, ast_vtype::String } {}
};

struct TypedASTBooleanLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text () { return m_token->m_text; }

	TypedASTBooleanLiteral() : 
        TypedAST{ ast_type::BooleanLiteral, ast_vtype::Boolean } {}
};

struct TypedASTNullLiteral : public TypedAST {

	TypedASTNullLiteral() : 
        TypedAST{ ast_type::NullLiteral, ast_vtype::Null } {}
};

struct TypedASTObjectLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

    // future feature 
    // the value type for objects must be followeb by a class identifier
	TypedASTObjectLiteral() : 
        TypedAST{ ast_type::ObjectLiteral, ast_vtype::Object } {}
};

struct TypedASTArrayLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_elements;

	TypedASTArrayLiteral() : TypedAST{ ast_type::ArrayLiteral } {}
};

struct TypedASTDictionaryLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	TypedASTDictionaryLiteral() : 
        TypedAST{ ast_type::DictionaryLiteral, ast_vtype::Dictionary } {}
};

struct TypedASTFunctionLiteral : public TypedAST {
	std::unique_ptr<TypedAST> m_body;
	std::vector<std::unique_ptr<TypedAST>> m_args;
	std::vector<std::string> m_captures;

	TypedASTFunctionLiteral() : 
        TypedAST{ ast_type::FunctionLiteral } {}
};

// doesnt have a ast_vtype
struct TypedASTDeclarationList : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_declarations;

	TypedASTDeclarationList() : TypedAST{ ast_type::DeclarationList } {}
};

// doesnt have a ast_vtype
struct TypedASTDeclaration : public TypedAST {
	Token const* m_identifier_token;
	Token const* m_typename_token { nullptr }; // can be nullptr
	std::unique_ptr<TypedAST> m_value; // can be nullptr

	std::string const& identifier_text() const { return m_identifier_token->m_text; }
	std::string const& typename_text() const { return m_typename_token->m_text; }

	TypedASTDeclaration() : TypedAST{ ast_type::Declaration } {}
};

// the ast_vtype must be computed
struct TypedASTIdentifier : public TypedAST {
	Token const* m_token;
	TypedASTDeclaration* m_declaration { nullptr };

	std::string const& text () { return m_token->m_text; }

	TypedASTIdentifier() : TypedAST{ ast_type::Identifier } {}
};

// the value depends on the operator
struct TypedASTBinaryExpression : public TypedAST {
	token_type m_op;
	std::unique_ptr<TypedAST> m_lhs;
	std::unique_ptr<TypedAST> m_rhs;

	TypedASTBinaryExpression() : TypedAST{ ast_type::BinaryExpression } {}
};

// the value depends on the return value of callee
struct TypedASTCallExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_callee;
	std::vector<std::unique_ptr<TypedAST>> m_args;

	TypedASTCallExpression() : TypedAST{ ast_type::CallExpression } {}
};

struct TypedASTIndexExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_callee;
	std::unique_ptr<TypedAST> m_index;

	TypedASTIndexExpression() : TypedAST{ ast_type::IndexExpression } {}
};

struct TypedASTBlock : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	TypedASTBlock() : TypedAST{ ast_type::Block } {}
};

struct TypedASTReturnStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_value;

	TypedASTReturnStatement() : TypedAST{ ast_type::ReturnStatement } {}
};

struct TypedASTIfStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_body;

	TypedASTIfStatement() : TypedAST{ ast_type::IfStatement } {}
};

struct TypedASTForStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_declaration;
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_action;
	std::unique_ptr<TypedAST> m_body;

	TypedASTForStatement() : TypedAST{ ast_type::ForStatement } {}
};
