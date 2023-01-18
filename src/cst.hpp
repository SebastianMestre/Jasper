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

struct Expr : public CST {
	Expr(CSTTag tag)
	    : CST {tag} {}
};

struct Stmt : public CST {
	Stmt(CSTTag tag)
	    : CST {tag} {}
};

struct Block;

struct DeclarationData {
	Token const* m_identifier_token;
	CST* m_type_hint {nullptr};  // can be nullptr
	CST* m_value {nullptr}; // can be nullptr

	InternedString const& identifier() const {
		return m_identifier_token->m_text;
	}
};

using FuncParameters = std::vector<DeclarationData>;

struct Declaration : public Stmt {
	// This function is very cold -- it's ok to use virtuals
	virtual InternedString const& identifier_virtual() const = 0;

	Declaration(CSTTag tag)
		: Stmt {tag} {}
};

struct PlainDeclaration : public Declaration {
	DeclarationData m_data;

	InternedString const& identifier() const {
		return m_data.identifier();
	}

	InternedString const& identifier_virtual() const override {
		return identifier();
	}

	PlainDeclaration(DeclarationData data)
	    : Declaration {CSTTag::PlainDeclaration}
	    , m_data {std::move(data)} {}
};

struct FuncDeclaration : public Declaration {
	Token const* m_identifier;
	FuncParameters m_args;
	CST* m_body;

	InternedString const& identifier() const {
		return m_identifier->m_text;
	}

	InternedString const& identifier_virtual() const override {
		return identifier();
	}

	FuncDeclaration(Token const* identifier, FuncParameters args, CST* body)
	    : Declaration {CSTTag::FuncDeclaration}
	    , m_identifier {identifier}
	    , m_args {std::move(args)}
	    , m_body {body} {}
};

struct BlockFuncDeclaration : public Declaration {
	Token const* m_identifier;
	FuncParameters m_args;
	Block* m_body;

	InternedString const& identifier() const {
		return m_identifier->m_text;
	}

	InternedString const& identifier_virtual() const override {
		return identifier();
	}

	BlockFuncDeclaration(Token const* identifier, FuncParameters args, Block* body)
	    : Declaration {CSTTag::BlockFuncDeclaration}
	    , m_identifier {identifier}
	    , m_args {std::move(args)}
	    , m_body {body} {}
};

struct Program : public CST {
	std::vector<Declaration*> m_declarations;

	Program(std::vector<Declaration*> declarations)
	    : CST {CSTTag::Program}
	    , m_declarations {std::move(declarations)} {}
};

struct IntegerLiteral : public CST {
	bool m_negative {false};
	Token const* m_sign {nullptr};
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	IntegerLiteral(bool negative, Token const* sign, Token const* token)
	    : CST {CSTTag::IntegerLiteral}
	    , m_negative {negative}
	    , m_sign {sign}
	    , m_token {token} {}
};

struct NumberLiteral : public CST {
	bool m_negative {false};
	Token const* m_sign {nullptr};
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	NumberLiteral(bool negative, Token const* sign, Token const* token)
	    : CST {CSTTag::NumberLiteral}
	    , m_negative {negative}
	    , m_sign {sign}
	    , m_token {token} {}
};

struct StringLiteral : public CST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	StringLiteral(Token const* token)
	    : CST {CSTTag::StringLiteral}
	    , m_token {token} {}
};

struct BooleanLiteral : public CST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	BooleanLiteral(Token const* token)
	    : CST {CSTTag::BooleanLiteral}
	    , m_token {token} {}
};

struct NullLiteral : public CST {

	NullLiteral()
	    : CST {CSTTag::NullLiteral} {}
};

struct ArrayLiteral : public CST {
	std::vector<CST*> m_elements;

	ArrayLiteral(std::vector<CST*> elements)
	    : CST {CSTTag::ArrayLiteral}
	    , m_elements {std::move(elements)} {}
};

struct BlockFunctionLiteral : public CST {
	Block* m_body;
	FuncParameters m_args;

	BlockFunctionLiteral(Block* body, FuncParameters args)
	    : CST {CSTTag::BlockFunctionLiteral}
	    , m_body {body}
	    , m_args {std::move(args)} {}
};

struct FunctionLiteral : public CST {
	CST* m_body;
	FuncParameters m_args;

	FunctionLiteral(CST* body, FuncParameters args)
	    : CST {CSTTag::FunctionLiteral}
	    , m_body {body}
	    , m_args {std::move(args)} {}
};

struct Identifier : public CST {
	Token const* m_token;

	InternedString const& text() {
		return m_token->m_text;
	}

	Identifier(Token const* token)
	    : CST {CSTTag::Identifier}
	    , m_token {token} {}
};

struct BinaryExpression : public CST {
	Token const* m_op_token;
	CST* m_lhs;
	CST* m_rhs;

	BinaryExpression(Token const* op_token, CST* lhs, CST* rhs)
	    : CST {CSTTag::BinaryExpression}
	    , m_op_token {op_token}
	    , m_lhs {lhs}
	    , m_rhs {rhs} {}
};

struct CallExpression : public CST {
	CST* m_callee;
	std::vector<CST*> m_args;

	CallExpression(CST* callee, std::vector<CST*> args)
	    : CST {CSTTag::CallExpression}
	    , m_callee {callee}
	    , m_args {std::move(args)} {}
};

struct IndexExpression : public CST {
	CST* m_callee;
	CST* m_index;

	IndexExpression(CST* callee, CST* index)
	    : CST {CSTTag::IndexExpression}
	    , m_callee {callee}
	    , m_index {index} {}
};

struct AccessExpression : public CST {
	CST* m_record;
	Token const* m_member;

	AccessExpression(CST* record, Token const* member)
	    : CST {CSTTag::AccessExpression}
	    , m_record {record}
	    , m_member {member} {}
};

struct TernaryExpression : public CST {
	CST* m_condition;
	CST* m_then_expr;
	CST* m_else_expr;

	TernaryExpression(CST* condition, CST* then_expr, CST* else_expr)
	    : CST {CSTTag::TernaryExpression}
	    , m_condition {condition}
	    , m_then_expr {then_expr}
	    , m_else_expr {else_expr} {}
};

struct MatchExpression : public CST {
	struct CaseData {
		Token const* m_name;
		Token const* m_identifier;
		CST* m_type_hint {nullptr};
		CST* m_expression;
	};

	// TODO: allow matching on arbitrary expressions
	Identifier m_matchee {nullptr};
	CST* m_type_hint {nullptr};
	std::vector<CaseData> m_cases;

	MatchExpression(Identifier matchee, CST* type_hint, std::vector<CaseData> cases)
	    : CST {CSTTag::MatchExpression}
	    , m_matchee {matchee}
	    , m_type_hint {type_hint}
	    , m_cases {cases} {}
};

struct ConstructorExpression : public CST {
	CST* m_constructor;
	std::vector<CST*> m_args;

	ConstructorExpression(CST* constructor, std::vector<CST*> args)
	    : CST {CSTTag::ConstructorExpression}
	    , m_constructor {constructor}
	    , m_args {std::move(args)} {}
};


struct SequenceExpression : public CST {
	Block* m_body;

	SequenceExpression(Block* body)
	    : CST {CSTTag::SequenceExpression}
	    , m_body {body} {}
};

struct Block : public Stmt {
	std::vector<CST*> m_body;

	Block(std::vector<CST*> body)
	    : Stmt {CSTTag::Block}
	    , m_body {body} {}
};

struct ReturnStatement : public Stmt {
	CST* m_value;

	ReturnStatement(CST* value)
	    : Stmt {CSTTag::ReturnStatement}
	    , m_value {value} {}
};

struct IfElseStatement : public Stmt {
	CST* m_condition;
	CST* m_body;
	CST* m_else_body {nullptr}; // can be nullptr

	IfElseStatement(CST* condition, CST* body, CST* else_body)
	    : Stmt {CSTTag::IfElseStatement}
	    , m_condition {condition}
	    , m_body {body}
	    , m_else_body {else_body} {}
};

struct ForStatement : public Stmt {
	DeclarationData m_declaration;
	CST* m_condition;
	CST* m_action;
	CST* m_body;

	ForStatement(DeclarationData declaration, CST* condition, CST* action, CST* body)
	    : Stmt {CSTTag::ForStatement}
	    , m_declaration {std::move(declaration)}
	    , m_condition {condition}
	    , m_action {action}
	    , m_body {body} {}
};

struct WhileStatement : public Stmt {
	CST* m_condition;
	CST* m_body;

	WhileStatement(CST* condition, CST* body)
	    : Stmt {CSTTag::WhileStatement}
	    , m_condition {condition}
	    , m_body {body} {}
};

struct TypeTerm : public CST {
	CST* m_callee;
	std::vector<CST*> m_args;

	TypeTerm(CST* callee, std::vector<CST*> args)
	    : CST {CSTTag::TypeTerm}
	    , m_callee {callee}
	    , m_args {std::move(args)} {}
};

// A TypeVar is a name, bound to a type variable of any kind.
// e.g. a type function, a polytype or a monotype
struct TypeVar : public CST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	TypeVar(Token const* token)
	    : CST {CSTTag::TypeVar}
	    , m_token {token} {}
};

struct UnionExpression : public CST {
	// TODO: better storage?
	std::vector<Identifier> m_constructors;
	std::vector<CST*> m_types;

	UnionExpression(std::vector<Identifier> constructors, std::vector<CST*> types)
	    : CST {CSTTag::UnionExpression}
	    , m_constructors {std::move(constructors)}
	    , m_types {std::move(types)} {}
};

struct StructExpression : public CST {
	// TODO: better storage?
	std::vector<Identifier> m_fields;
	std::vector<CST*> m_types;

	StructExpression(std::vector<Identifier> fields, std::vector<CST*> types)
	    : CST {CSTTag::StructExpression}
	    , m_fields {std::move(fields)}
	    , m_types {std::move(types)} {}
};

void print(CST*, int);

} // namespace CST
