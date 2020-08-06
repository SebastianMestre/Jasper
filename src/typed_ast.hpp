#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "ast_type.hpp"
#include "token.hpp"
#include "token_type.hpp"
#include "value_type.hpp"
#include "typesystem_types.hpp"

namespace AST {
struct AST;
}

namespace TypedAST {

struct TypedAST {
protected:
	ast_type m_type;

public:
	TypedAST(ast_type type) : m_type { type } {}

	MonoId m_value_type;
	ast_type type() const { return m_type; }
	virtual ~TypedAST() = default;
};

TypedAST* convertAST(AST::AST*);
std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST::AST>&);

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct NumberLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() { return m_token->m_text; }

	NumberLiteral() : TypedAST { ast_type::NumberLiteral } {}
};

struct StringLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() { return m_token->m_text; }

	StringLiteral() : TypedAST { ast_type::StringLiteral } {}
};

struct BooleanLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() { return m_token->m_text; }

	BooleanLiteral() : TypedAST { ast_type::BooleanLiteral } {}
};

struct NullLiteral : public TypedAST {

	NullLiteral() : TypedAST { ast_type::NullLiteral } {}
};

struct ObjectLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	// future feature
	// the value type for objects must be followeb by a class identifier
	ObjectLiteral() : TypedAST { ast_type::ObjectLiteral } {}
};

struct ArrayLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_elements;

	ArrayLiteral() : TypedAST { ast_type::ArrayLiteral } {}
};

struct DictionaryLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	DictionaryLiteral() : TypedAST { ast_type::DictionaryLiteral } {}
};

struct FunctionLiteral : public TypedAST {
	std::unique_ptr<TypedAST> m_body;
	std::vector<std::unique_ptr<TypedAST>> m_args;
	std::unordered_set<std::string> m_captures;

	FunctionLiteral() : TypedAST { ast_type::FunctionLiteral } {}
};

// doesnt have a ast_vtype
struct DeclarationList : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_declarations;

	DeclarationList() : TypedAST { ast_type::DeclarationList } {}
};

struct Declaration : public TypedAST {
	Token const* m_identifier_token;
	std::unique_ptr<TypedAST> m_value; // can be nullptr

	// nullptr means global
	FunctionLiteral* m_surrounding_function { nullptr };

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}

	Declaration() : TypedAST { ast_type::Declaration } {}
};

// the ast_vtype must be computed
struct Identifier : public TypedAST {
	Token const* m_token;
	Declaration* m_declaration { nullptr };

	std::string const& text() { return m_token->m_text; }

	Identifier() : TypedAST { ast_type::Identifier } {}
};

// the value depends on the return value of callee
struct CallExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_callee;
	std::vector<std::unique_ptr<TypedAST>> m_args;

	CallExpression() : TypedAST { ast_type::CallExpression } {}
};

struct IndexExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_callee;
	std::unique_ptr<TypedAST> m_index;

	IndexExpression() : TypedAST { ast_type::IndexExpression } {}
};

struct Block : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	Block() : TypedAST { ast_type::Block } {}
};

struct ReturnStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_value;

	ReturnStatement() : TypedAST { ast_type::ReturnStatement } {}
};

struct IfStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_body;

	IfStatement() : TypedAST { ast_type::IfStatement } {}
};

struct ForStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_declaration;
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_action;
	std::unique_ptr<TypedAST> m_body;

	ForStatement() : TypedAST { ast_type::ForStatement } {}
};

} // namespace TypedAST
