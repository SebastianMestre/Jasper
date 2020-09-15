#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast_type.hpp"
#include "token.hpp"

namespace AST {

struct AST {
  protected:
	ASTType m_type;

  public:
	AST() = default;
	AST(ASTType type)
	    : m_type {type} {}

	ASTType type() const {
		return m_type;
	}
	virtual ~AST() = default;
};

struct IntegerLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	IntegerLiteral()
	    : AST {ASTType::IntegerLiteral} {}
};

struct NumberLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	NumberLiteral()
	    : AST {ASTType::NumberLiteral} {}
};

struct StringLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	StringLiteral()
	    : AST {ASTType::StringLiteral} {}
};

struct BooleanLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	BooleanLiteral()
	    : AST {ASTType::BooleanLiteral} {}
};

struct NullLiteral : public AST {

	NullLiteral()
	    : AST {ASTType::NullLiteral} {}
};

struct ObjectLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ObjectLiteral()
	    : AST {ASTType::ObjectLiteral} {}
};

struct ArrayLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_elements;

	ArrayLiteral()
	    : AST {ASTType::ArrayLiteral} {}
};

struct DictionaryLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	DictionaryLiteral()
	    : AST {ASTType::DictionaryLiteral} {}
};

struct FunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	FunctionLiteral()
	    : AST {ASTType::FunctionLiteral} {}
};

struct ShortFunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	ShortFunctionLiteral()
	    : AST {ASTType::ShortFunctionLiteral} {}
};

struct DeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	DeclarationList()
	    : AST {ASTType::DeclarationList} {}
};

struct Declaration : public AST {
	Token const* m_identifier_token;
	std::unique_ptr<AST> m_type;  // can be nullptr
	std::unique_ptr<AST> m_value; // can be nullptr

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}

	Declaration()
	    : AST {ASTType::Declaration} {}
};

struct Identifier : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : AST {ASTType::Identifier} {}
};

struct BinaryExpression : public AST {
	Token const* m_op_token;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	BinaryExpression()
	    : AST {ASTType::BinaryExpression} {}
};

struct CallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::vector<std::unique_ptr<AST>> m_args;

	CallExpression()
	    : AST {ASTType::CallExpression} {}
};

struct IndexExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_index;

	IndexExpression()
	    : AST {ASTType::IndexExpression} {}
};

struct TernaryExpression : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_then_expr;
	std::unique_ptr<AST> m_else_expr;

	TernaryExpression()
	    : AST {ASTType::TernaryExpression} {}
};

struct Block : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	Block()
	    : AST {ASTType::Block} {}
};

struct ReturnStatement : public AST {
	std::unique_ptr<AST> m_value;

	ReturnStatement()
	    : AST {ASTType::ReturnStatement} {}
};

struct IfElseStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;
	std::unique_ptr<AST> m_else_body; // can be nullptr

	IfElseStatement()
	    : AST {ASTType::IfElseStatement} {}
};

struct ForStatement : public AST {
	std::unique_ptr<AST> m_declaration;
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_action;
	std::unique_ptr<AST> m_body;

	ForStatement()
	    : AST {ASTType::ForStatement} {}
};

struct WhileStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;

	WhileStatement()
	    : AST {ASTType::WhileStatement} {}
};

struct TypeTerm : public AST {
	std::unique_ptr<AST> m_callee;
	std::vector<std::unique_ptr<AST>> m_args;

	TypeTerm()
	    : AST {ASTType::TypeTerm} {}
};

void print(AST*, int);

} // namespace AST
