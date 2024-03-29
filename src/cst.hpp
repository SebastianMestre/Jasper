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

struct Declaration;

struct DeclarationData {
	Token const* m_identifier_token;
	Expr* m_type_hint {nullptr};  // can be nullptr
	Expr* m_value {nullptr}; // can be nullptr

	InternedString const& identifier() const {
		return m_identifier_token->m_text;
	}
};

using FuncParameters = std::vector<DeclarationData>;


struct Program : public CST {
	std::vector<Declaration*> m_declarations;

	Program(std::vector<Declaration*> declarations)
	    : CST {CSTTag::Program}
	    , m_declarations {std::move(declarations)} {}
};

// Expression language

struct IntegerLiteral : public Expr {
	bool m_negative {false};
	Token const* m_sign {nullptr};
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	IntegerLiteral(bool negative, Token const* sign, Token const* token)
	    : Expr {CSTTag::IntegerLiteral}
	    , m_negative {negative}
	    , m_sign {sign}
	    , m_token {token} {}
};

struct NumberLiteral : public Expr {
	bool m_negative {false};
	Token const* m_sign {nullptr};
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	NumberLiteral(bool negative, Token const* sign, Token const* token)
	    : Expr {CSTTag::NumberLiteral}
	    , m_negative {negative}
	    , m_sign {sign}
	    , m_token {token} {}
};

struct StringLiteral : public Expr {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	StringLiteral(Token const* token)
	    : Expr {CSTTag::StringLiteral}
	    , m_token {token} {}
};

struct BooleanLiteral : public Expr {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	BooleanLiteral(Token const* token)
	    : Expr {CSTTag::BooleanLiteral}
	    , m_token {token} {}
};

struct NullLiteral : public Expr {

	NullLiteral()
	    : Expr {CSTTag::NullLiteral} {}
};

struct ArrayLiteral : public Expr {
	std::vector<Expr*> m_elements;

	ArrayLiteral(std::vector<Expr*> elements)
	    : Expr {CSTTag::ArrayLiteral}
	    , m_elements {std::move(elements)} {}
};

struct BlockFunctionLiteral : public Expr {
	Block* m_body;
	FuncParameters m_args;

	BlockFunctionLiteral(Block* body, FuncParameters args)
	    : Expr {CSTTag::BlockFunctionLiteral}
	    , m_body {body}
	    , m_args {std::move(args)} {}
};

struct FunctionLiteral : public Expr {
	Expr* m_body;
	FuncParameters m_args;

	FunctionLiteral(Expr* body, FuncParameters args)
	    : Expr {CSTTag::FunctionLiteral}
	    , m_body {body}
	    , m_args {std::move(args)} {}
};

struct Identifier : public Expr {
	Token const* m_token;

	InternedString const& text() {
		return m_token->m_text;
	}

	Identifier(Token const* token)
	    : Expr {CSTTag::Identifier}
	    , m_token {token} {}
};

struct BinaryExpression : public Expr {
	Token const* m_op_token;
	Expr* m_lhs;
	Expr* m_rhs;

	BinaryExpression(Token const* op_token, Expr* lhs, Expr* rhs)
	    : Expr {CSTTag::BinaryExpression}
	    , m_op_token {op_token}
	    , m_lhs {lhs}
	    , m_rhs {rhs} {}
};

struct CallExpression : public Expr {
	Expr* m_callee;
	std::vector<Expr*> m_args;

	CallExpression(Expr* callee, std::vector<Expr*> args)
	    : Expr {CSTTag::CallExpression}
	    , m_callee {callee}
	    , m_args {std::move(args)} {}
};

struct IndexExpression : public Expr {
	Expr* m_callee;
	Expr* m_index;

	IndexExpression(Expr* callee, Expr* index)
	    : Expr {CSTTag::IndexExpression}
	    , m_callee {callee}
	    , m_index {index} {}
};

struct AccessExpression : public Expr {
	Expr* m_record;
	Token const* m_member;

	AccessExpression(Expr* record, Token const* member)
	    : Expr {CSTTag::AccessExpression}
	    , m_record {record}
	    , m_member {member} {}
};

struct TernaryExpression : public Expr {
	Expr* m_condition;
	Expr* m_then_expr;
	Expr* m_else_expr;

	TernaryExpression(Expr* condition, Expr* then_expr, Expr* else_expr)
	    : Expr {CSTTag::TernaryExpression}
	    , m_condition {condition}
	    , m_then_expr {then_expr}
	    , m_else_expr {else_expr} {}
};

struct MatchExpression : public Expr {
	struct CaseData {
		Token const* m_name;
		Token const* m_identifier;
		Expr* m_type_hint {nullptr};
		Expr* m_expression;
	};

	// TODO: allow matching on arbitrary expressions
	Identifier m_matchee {nullptr};
	Expr* m_type_hint {nullptr};
	std::vector<CaseData> m_cases;

	MatchExpression(Identifier matchee, Expr* type_hint, std::vector<CaseData> cases)
	    : Expr {CSTTag::MatchExpression}
	    , m_matchee {matchee}
	    , m_type_hint {type_hint}
	    , m_cases {cases} {}
};

struct ConstructorExpression : public Expr {
	Expr* m_constructor;
	std::vector<Expr*> m_args;

	ConstructorExpression(Expr* constructor, std::vector<Expr*> args)
	    : Expr {CSTTag::ConstructorExpression}
	    , m_constructor {constructor}
	    , m_args {std::move(args)} {}
};

struct SequenceExpression : public Expr {
	Block* m_body;

	SequenceExpression(Block* body)
	    : Expr {CSTTag::SequenceExpression}
	    , m_body {body} {}
};

// Statement language

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
	Expr* m_body;

	InternedString const& identifier() const {
		return m_identifier->m_text;
	}

	InternedString const& identifier_virtual() const override {
		return identifier();
	}

	FuncDeclaration(Token const* identifier, FuncParameters args, Expr* body)
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

struct Block : public Stmt {
	std::vector<Stmt*> m_body;

	Block(std::vector<Stmt*> body)
	    : Stmt {CSTTag::Block}
	    , m_body {body} {}
};

struct ReturnStatement : public Stmt {
	Expr* m_value;

	ReturnStatement(Expr* value)
	    : Stmt {CSTTag::ReturnStatement}
	    , m_value {value} {}
};

struct IfElseStatement : public Stmt {
	Expr* m_condition;
	Stmt* m_body;
	Stmt* m_else_body {nullptr}; // can be nullptr

	IfElseStatement(Expr* condition, Stmt* body, Stmt* else_body)
	    : Stmt {CSTTag::IfElseStatement}
	    , m_condition {condition}
	    , m_body {body}
	    , m_else_body {else_body} {}
};

struct ForStatement : public Stmt {
	DeclarationData m_declaration;
	Expr* m_condition;
	Expr* m_action;
	Stmt* m_body;

	ForStatement(DeclarationData declaration, Expr* condition, Expr* action, Stmt* body)
	    : Stmt {CSTTag::ForStatement}
	    , m_declaration {std::move(declaration)}
	    , m_condition {condition}
	    , m_action {action}
	    , m_body {body} {}
};

struct WhileStatement : public Stmt {
	Expr* m_condition;
	Stmt* m_body;

	WhileStatement(Expr* condition, Stmt* body)
	    : Stmt {CSTTag::WhileStatement}
	    , m_condition {condition}
	    , m_body {body} {}
};

struct ExpressionStatement : public Stmt {
	Expr* m_expression;

	ExpressionStatement(Expr* expression)
	    : Stmt {CSTTag::ExpressionStatement}
	    , m_expression {expression} {}
};

// Type language

struct TypeTerm : public Expr {
	Expr* m_callee;
	std::vector<Expr*> m_args;

	TypeTerm(Expr* callee, std::vector<Expr*> args)
	    : Expr {CSTTag::TypeTerm}
	    , m_callee {callee}
	    , m_args {std::move(args)} {}
};

// A TypeVar is a name, bound to a type variable of any kind.
// e.g. a type function, a polytype or a monotype
struct TypeVar : public Expr {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	TypeVar(Token const* token)
	    : Expr {CSTTag::TypeVar}
	    , m_token {token} {}
};

struct UnionExpression : public Expr {
	// TODO: better storage?
	std::vector<Identifier> m_constructors;
	std::vector<Expr*> m_types;

	UnionExpression(std::vector<Identifier> constructors, std::vector<Expr*> types)
	    : Expr {CSTTag::UnionExpression}
	    , m_constructors {std::move(constructors)}
	    , m_types {std::move(types)} {}
};

struct StructExpression : public Expr {
	// TODO: better storage?
	std::vector<Identifier> m_fields;
	std::vector<Expr*> m_types;

	StructExpression(std::vector<Identifier> fields, std::vector<Expr*> types)
	    : Expr {CSTTag::StructExpression}
	    , m_fields {std::move(fields)}
	    , m_types {std::move(types)} {}
};

void print(CST*, int);

} // namespace CST
