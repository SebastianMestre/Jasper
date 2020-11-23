#pragma once

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

struct Declaration : public AST {
	Token const* m_identifier_token;
	AST* m_type_hint {nullptr};  // can be nullptr
	AST* m_value {nullptr}; // can be nullptr

	std::string const& identifier_text() const {
		return m_identifier_token->m_text.str();
	}

	Declaration()
	    : AST {ASTTag::Declaration} {}
};

struct DeclarationList : public AST {
	std::vector<Declaration> m_declarations;

	DeclarationList()
	    : AST {ASTTag::DeclarationList} {}
};

struct IntegerLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	IntegerLiteral()
	    : AST {ASTTag::IntegerLiteral} {}
};

struct NumberLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	NumberLiteral()
	    : AST {ASTTag::NumberLiteral} {}
};

struct StringLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	StringLiteral()
	    : AST {ASTTag::StringLiteral} {}
};

struct BooleanLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	BooleanLiteral()
	    : AST {ASTTag::BooleanLiteral} {}
};

struct NullLiteral : public AST {

	NullLiteral()
	    : AST {ASTTag::NullLiteral} {}
};

struct ObjectLiteral : public AST {
	std::vector<Declaration> m_body;

	ObjectLiteral()
	    : AST {ASTTag::ObjectLiteral} {}
};

struct ArrayLiteral : public AST {
	std::vector<AST*> m_elements;

	ArrayLiteral()
	    : AST {ASTTag::ArrayLiteral} {}
};

struct DictionaryLiteral : public AST {
	std::vector<Declaration> m_body;

	DictionaryLiteral()
	    : AST {ASTTag::DictionaryLiteral} {}
};

struct FunctionLiteral : public AST {
	AST* m_body;
	std::vector<Declaration> m_args;

	FunctionLiteral()
	    : AST {ASTTag::FunctionLiteral} {}
};

struct ShortFunctionLiteral : public AST {
	AST* m_body;
	std::vector<Declaration> m_args;

	ShortFunctionLiteral()
	    : AST {ASTTag::ShortFunctionLiteral} {}
};

struct Identifier : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	Identifier()
	    : AST {ASTTag::Identifier} {}
};

struct BinaryExpression : public AST {
	Token const* m_op_token;
	AST* m_lhs;
	AST* m_rhs;

	BinaryExpression()
	    : AST {ASTTag::BinaryExpression} {}
};

struct CallExpression : public AST {
	AST* m_callee;
	std::vector<AST*> m_args;

	CallExpression()
	    : AST {ASTTag::CallExpression} {}
};

struct IndexExpression : public AST {
	AST* m_callee;
	AST* m_index;

	IndexExpression()
	    : AST {ASTTag::IndexExpression} {}
};

struct AccessExpression : public AST {
	AST* m_object;
	Token const* m_member;

	AccessExpression()
	    : AST {ASTTag::AccessExpression} {}
};

struct TernaryExpression : public AST {
	AST* m_condition;
	AST* m_then_expr;
	AST* m_else_expr;

	TernaryExpression()
	    : AST {ASTTag::TernaryExpression} {}
};

struct MatchExpression : public AST {
	// NOTE: I'm avoiding using Declaration for this
	// because it will generate confusion
	Identifier m_matchee;
	AST* m_type_hint {nullptr};
	std::vector<Token const*> m_cases;
	std::vector<AST*> m_expressions;

	MatchExpression()
	    : AST {ASTTag::MatchExpression} {}
};

struct ConstructorExpression : public AST {
	AST* m_constructor;
	std::vector<AST*> m_args;

	ConstructorExpression()
	    : AST {ASTTag::ConstructorExpression} {}
};

struct Block : public AST {
	std::vector<AST*> m_body;

	Block()
	    : AST {ASTTag::Block} {}
};

struct ReturnStatement : public AST {
	AST* m_value;

	ReturnStatement()
	    : AST {ASTTag::ReturnStatement} {}
};

struct IfElseStatement : public AST {
	AST* m_condition;
	AST* m_body;
	AST* m_else_body {nullptr}; // can be nullptr

	IfElseStatement()
	    : AST {ASTTag::IfElseStatement} {}
};

struct ForStatement : public AST {
	Declaration m_declaration;
	AST* m_condition;
	AST* m_action;
	AST* m_body;

	ForStatement()
	    : AST {ASTTag::ForStatement} {}
};

struct WhileStatement : public AST {
	AST* m_condition;
	AST* m_body;

	WhileStatement()
	    : AST {ASTTag::WhileStatement} {}
};

struct TypeTerm : public AST {
	AST* m_callee;
	std::vector<AST*> m_args;

	TypeTerm()
	    : AST {ASTTag::TypeTerm} {}
};

// A TypeVar is a name, bound to a type variable of any kind.
// e.g. a type function, a polytype or a monotype
struct TypeVar : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	TypeVar()
	    : AST {ASTTag::TypeVar} {}
};

struct UnionExpression : public AST {
	// TODO: better storage?
	std::vector<Identifier> m_constructors;
	std::vector<AST*> m_types;

	UnionExpression()
	    : AST {ASTTag::UnionExpression} {}
};

struct TupleExpression : public AST {
	std::vector<AST*> m_types;

	TupleExpression()
	    : AST {ASTTag::TupleExpression} {}
};

struct StructExpression : public AST {
	// TODO: better storage?
	std::vector<Identifier> m_fields;
	std::vector<AST*> m_types;

	StructExpression()
	    : AST {ASTTag::StructExpression} {}
};

void print(AST*, int);

} // namespace AST
