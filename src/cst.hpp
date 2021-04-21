#pragma once

#include <string>
#include <vector>
#include <cassert>

#include "./utils/interned_string.hpp"
#include "cst_tag.hpp"
#include "token.hpp"

namespace CST {

struct CST {
  protected:
	CSTTag m_type;

  public:
	CST() = default;
	CST(CSTTag type)
	    : m_type {type} {}

	CSTTag type() const {
		return m_type;
	}
	virtual ~CST() = default;
};

struct DeclarationData {
	Token const* m_identifier_token;
	CST* m_type_hint {nullptr};  // can be nullptr
	CST* m_value {nullptr}; // can be nullptr

	InternedString const& identifier() const {
		return m_identifier_token->m_text;
	}
};

using FuncArguments = std::vector<DeclarationData>;

struct Declaration : public CST {
	// This function is very cold -- it's ok to use virtuals
	virtual InternedString const& identifier_virtual() const = 0;

	Declaration(CSTTag tag)
		: CST {tag} {}
};

struct PlainDeclaration : public Declaration {
	DeclarationData m_data;

	InternedString const& identifier() const {
		return m_data.identifier();
	}

	InternedString const& identifier_virtual() const override {
		return identifier();
	}

	PlainDeclaration()
	    : Declaration {CSTTag::PlainDeclaration} {}
};

struct FuncDeclaration : public Declaration {
	Token const* m_identifier;
	FuncArguments m_args;
	CST* m_body;

	InternedString const& identifier() const {
		return m_identifier->m_text;
	}

	InternedString const& identifier_virtual() const override {
		return identifier();
	}

	FuncDeclaration()
	    : Declaration {CSTTag::FuncDeclaration} {}
};

struct DeclarationList : public CST {
	std::vector<Declaration*> m_declarations;

	DeclarationList()
	    : CST {CSTTag::DeclarationList} {}
};

struct IntegerLiteral : public CST {
	bool m_negative {false};
	Token const* m_sign {nullptr};
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	IntegerLiteral()
	    : CST {CSTTag::IntegerLiteral} {}
};

struct NumberLiteral : public CST {
	bool m_negative {false};
	Token const* m_sign {nullptr};
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	NumberLiteral()
	    : CST {CSTTag::NumberLiteral} {}
};

struct StringLiteral : public CST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	StringLiteral()
	    : CST {CSTTag::StringLiteral} {}
};

struct BooleanLiteral : public CST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	BooleanLiteral()
	    : CST {CSTTag::BooleanLiteral} {}
};

struct NullLiteral : public CST {

	NullLiteral()
	    : CST {CSTTag::NullLiteral} {}
};

struct ArrayLiteral : public CST {
	std::vector<CST*> m_elements;

	ArrayLiteral()
	    : CST {CSTTag::ArrayLiteral} {}
};

struct BlockFunctionLiteral : public CST {
	CST* m_body;
	FuncArguments m_args;

	BlockFunctionLiteral()
	    : CST {CSTTag::BlockFunctionLiteral} {}
};

struct FunctionLiteral : public CST {
	CST* m_body;
	FuncArguments m_args;

	FunctionLiteral()
	    : CST {CSTTag::FunctionLiteral} {}
};

struct Identifier : public CST {
	Token const* m_token;

	InternedString const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : CST {CSTTag::Identifier} {}
};

struct BinaryExpression : public CST {
	Token const* m_op_token;
	CST* m_lhs;
	CST* m_rhs;

	BinaryExpression()
	    : CST {CSTTag::BinaryExpression} {}
};

struct CallExpression : public CST {
	CST* m_callee;
	std::vector<CST*> m_args;

	CallExpression()
	    : CST {CSTTag::CallExpression} {}
};

struct IndexExpression : public CST {
	CST* m_callee;
	CST* m_index;

	IndexExpression()
	    : CST {CSTTag::IndexExpression} {}
};

struct AccessExpression : public CST {
	CST* m_record;
	Token const* m_member;

	AccessExpression()
	    : CST {CSTTag::AccessExpression} {}
};

struct TernaryExpression : public CST {
	CST* m_condition;
	CST* m_then_expr;
	CST* m_else_expr;

	TernaryExpression()
	    : CST {CSTTag::TernaryExpression} {}
};

struct MatchExpression : public CST {
	struct CaseData {
		Token const* m_name;
		Token const* m_identifier;
		CST* m_type_hint {nullptr};
		CST* m_expression;
	};

	// TODO: allow matching on arbitrary expressions
	Identifier m_matchee;
	CST* m_type_hint {nullptr};
	std::vector<CaseData> m_cases;

	MatchExpression()
	    : CST {CSTTag::MatchExpression} {}
};

struct ConstructorExpression : public CST {
	CST* m_constructor;
	std::vector<CST*> m_args;

	ConstructorExpression()
	    : CST {CSTTag::ConstructorExpression} {}
};

struct Block;

struct SequenceExpression : public CST {
	Block* m_body;

	SequenceExpression()
	    : CST {CSTTag::SequenceExpression} {}
};

struct Block : public CST {
	std::vector<CST*> m_body;

	Block()
	    : CST {CSTTag::Block} {}
};

struct ReturnStatement : public CST {
	CST* m_value;

	ReturnStatement()
	    : CST {CSTTag::ReturnStatement} {}
};

struct IfElseStatement : public CST {
	CST* m_condition;
	CST* m_body;
	CST* m_else_body {nullptr}; // can be nullptr

	IfElseStatement()
	    : CST {CSTTag::IfElseStatement} {}
};

struct ForStatement : public CST {
	DeclarationData m_declaration;
	CST* m_condition;
	CST* m_action;
	CST* m_body;

	ForStatement()
	    : CST {CSTTag::ForStatement} {}
};

struct WhileStatement : public CST {
	CST* m_condition;
	CST* m_body;

	WhileStatement()
	    : CST {CSTTag::WhileStatement} {}
};

struct TypeTerm : public CST {
	CST* m_callee;
	std::vector<CST*> m_args;

	TypeTerm()
	    : CST {CSTTag::TypeTerm} {}
};

// A TypeVar is a name, bound to a type variable of any kind.
// e.g. a type function, a polytype or a monotype
struct TypeVar : public CST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	TypeVar()
	    : CST {CSTTag::TypeVar} {}
};

struct UnionExpression : public CST {
	// TODO: better storage?
	std::vector<Identifier> m_constructors;
	std::vector<CST*> m_types;

	UnionExpression()
	    : CST {CSTTag::UnionExpression} {}
};

struct TupleExpression : public CST {
	std::vector<CST*> m_types;

	TupleExpression()
	    : CST {CSTTag::TupleExpression} {}
};

struct StructExpression : public CST {
	// TODO: better storage?
	std::vector<Identifier> m_fields;
	std::vector<CST*> m_types;

	StructExpression()
	    : CST {CSTTag::StructExpression} {}
};

void print(CST*, int);

} // namespace CST
