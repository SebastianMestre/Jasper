#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast_tag.hpp"
#include "token.hpp"

namespace AST {

struct AST {
  protected:
	ASTTag m_type;

  public:
	AST() = default;
	AST(ASTTag type)
	    : m_type {type} {}

	ASTTag type() const {
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
	    : AST {ASTTag::IntegerLiteral} {}
};

struct NumberLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	NumberLiteral()
	    : AST {ASTTag::NumberLiteral} {}
};

struct StringLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	StringLiteral()
	    : AST {ASTTag::StringLiteral} {}
};

struct BooleanLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	BooleanLiteral()
	    : AST {ASTTag::BooleanLiteral} {}
};

struct NullLiteral : public AST {

	NullLiteral()
	    : AST {ASTTag::NullLiteral} {}
};

struct ObjectLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ObjectLiteral()
	    : AST {ASTTag::ObjectLiteral} {}
};

struct ArrayLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_elements;

	ArrayLiteral()
	    : AST {ASTTag::ArrayLiteral} {}
};

struct DictionaryLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	DictionaryLiteral()
	    : AST {ASTTag::DictionaryLiteral} {}
};

struct FunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	FunctionLiteral()
	    : AST {ASTTag::FunctionLiteral} {}
};

struct ShortFunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	ShortFunctionLiteral()
	    : AST {ASTTag::ShortFunctionLiteral} {}
};

struct DeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	DeclarationList()
	    : AST {ASTTag::DeclarationList} {}
};

struct Declaration : public AST {
	Token const* m_identifier_token;
	std::unique_ptr<AST> m_type;  // can be nullptr
	std::unique_ptr<AST> m_value; // can be nullptr

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}

	Declaration()
	    : AST {ASTTag::Declaration} {}
};

struct Identifier : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : AST {ASTTag::Identifier} {}
};

struct BinaryExpression : public AST {
	Token const* m_op_token;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	BinaryExpression()
	    : AST {ASTTag::BinaryExpression} {}
};

struct CallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::vector<std::unique_ptr<AST>> m_args;

	CallExpression()
	    : AST {ASTTag::CallExpression} {}
};

struct IndexExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_index;

	IndexExpression()
	    : AST {ASTTag::IndexExpression} {}
};

struct TernaryExpression : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_then_expr;
	std::unique_ptr<AST> m_else_expr;

	TernaryExpression()
	    : AST {ASTTag::TernaryExpression} {}
};

struct Block : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	Block()
	    : AST {ASTTag::Block} {}
};

struct ReturnStatement : public AST {
	std::unique_ptr<AST> m_value;

	ReturnStatement()
	    : AST {ASTTag::ReturnStatement} {}
};

struct IfElseStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;
	std::unique_ptr<AST> m_else_body; // can be nullptr

	IfElseStatement()
	    : AST {ASTTag::IfElseStatement} {}
};

struct ForStatement : public AST {
	std::unique_ptr<AST> m_declaration;
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_action;
	std::unique_ptr<AST> m_body;

	ForStatement()
	    : AST {ASTTag::ForStatement} {}
};

struct WhileStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;

	WhileStatement()
	    : AST {ASTTag::WhileStatement} {}
};

struct TypeTerm : public AST {
	std::unique_ptr<AST> m_callee;
	std::vector<std::unique_ptr<AST>> m_args;

	TypeTerm()
	    : AST {ASTTag::TypeTerm} {}
};

// A TypeVar is a name, bound to a type variable of any kind.
// e.g. a type function, a polytype or a monotype
struct TypeVar : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	TypeVar()
	    : AST {ASTTag::TypeVar} {}
};

struct UnionExpression : public AST {
	// TODO: better storage?
	std::vector<std::unique_ptr<AST>> m_constructors;
	std::vector<std::unique_ptr<AST>> m_types;

	UnionExpression()
	    : AST {ASTTag::UnionExpression} {}
};

struct TupleExpression : public AST {
	std::vector<std::unique_ptr<AST>> m_types;

	TupleExpression()
	    : AST {ASTTag::TupleExpression} {}
};

struct StructExpression : public AST {
	// TODO: better storage?
	std::vector<std::unique_ptr<AST>> m_fields;
	std::vector<std::unique_ptr<AST>> m_types;

	StructExpression()
	    : AST {ASTTag::StructExpression} {}
};

void print(AST*, int);

} // namespace AST
