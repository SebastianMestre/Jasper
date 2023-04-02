#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <climits>

#include "./utils/interned_string.hpp"
#include "./typechecker/typechecker_types.hpp"
#include "ast_tag.hpp"

struct Token;

namespace CST {
struct CST;
}

namespace AST {

struct AST {
  protected:
	ASTTag m_type;

  public:
	AST(ASTTag type)
	    : m_type {type} {}

	CST::CST* m_cst {nullptr};

	ASTTag type() const { return m_type; }
	virtual ~AST() = default;
};

inline bool is_expression (AST* ast) {
	auto tag = ast->type();
	auto tag_idx = static_cast<int>(tag);
	return tag_idx < static_cast<int>(ASTTag::Block);
}

struct Expr : public AST {
	Expr(ASTTag type)
	    : AST {type} {}

	MetaTypeId m_meta_type {-1};
	MonoId m_value_type {-1};
};

struct Stmt : public AST {
	Stmt(ASTTag tag)
		: AST {tag} {}
};

struct FunctionLiteral;
struct SequenceExpression;

struct Declaration : public Stmt {
	InternedString m_identifier;

	Expr* m_type_hint {nullptr};  // can be nullptr
	Expr* m_value {nullptr}; // can be nullptr

	std::unordered_set<Declaration*> m_references;

	MetaTypeId m_meta_type {-1};
	bool m_is_polymorphic {false};
	MonoId m_value_type {-1}; // used for non-polymorphic decls, and during unification
	PolyId m_decl_type;

	int m_frame_offset {INT_MIN};

	FunctionLiteral* m_surrounding_function {nullptr};
	SequenceExpression* m_surrounding_seq_expr {nullptr};

	bool is_global() const {
		return !m_surrounding_function && !m_surrounding_seq_expr;
	}

	InternedString const& identifier_text() const;

	Declaration()
	    : Stmt {ASTTag::Declaration} {}
};

struct Program : public AST {
	std::vector<Declaration> m_declarations;

	Program()
	    : AST {ASTTag::Program} {}
};

struct NumberLiteral : public Expr {
	float m_value;

	float value() const {
		return m_value;
	}

	NumberLiteral()
	    : Expr {ASTTag::NumberLiteral} {}
};

struct IntegerLiteral : public Expr {
	int m_value;

	int value() const {
		return m_value;
	}

	IntegerLiteral()
	    : Expr {ASTTag::IntegerLiteral} {}
};

struct StringLiteral : public Expr {
	InternedString m_text;

	std::string const& text() {
		return m_text.str();
	}

	StringLiteral()
	    : Expr {ASTTag::StringLiteral} {}
};

struct BooleanLiteral : public Expr {
	bool m_value;

	BooleanLiteral()
	    : Expr {ASTTag::BooleanLiteral} {}
};

struct NullLiteral : public Expr {

	NullLiteral()
	    : Expr {ASTTag::NullLiteral} {}
};

struct ArrayLiteral : public Expr {
	std::vector<Expr*> m_elements;

	ArrayLiteral()
	    : Expr {ASTTag::ArrayLiteral} {}
};

struct FunctionLiteral : public Expr {
	struct CaptureData {
		Declaration* outer_declaration{nullptr};
		int outer_frame_offset{INT_MIN};
		int inner_frame_offset{INT_MIN};
	};

	Expr* m_body;
	std::vector<Declaration> m_args;
	std::unordered_map<InternedString, CaptureData> m_captures;
	FunctionLiteral* m_surrounding_function {nullptr};

	FunctionLiteral()
	    : Expr {ASTTag::FunctionLiteral} {}
};

struct Identifier : public Expr {
	enum class Origin { Global, Capture, Local };

	InternedString m_text;
	Declaration* m_declaration {nullptr}; // can be nullptr
	FunctionLiteral* m_surrounding_function {nullptr};

	Origin m_origin { Origin::Global };
	int m_frame_offset {INT_MIN};

	Token const* token() const;
	InternedString const& text() const {
		return m_text;
	}

	Identifier()
	    : Expr {ASTTag::Identifier} {}
};

struct CallExpression : public Expr {
	Expr* m_callee;
	std::vector<Expr*> m_args;

	CallExpression()
	    : Expr {ASTTag::CallExpression} {}
};

struct IndexExpression : public Expr {
	Expr* m_callee;
	Expr* m_index;

	IndexExpression()
	    : Expr {ASTTag::IndexExpression} {}
};

struct AccessExpression : public Expr {
	Expr* m_target;
	InternedString m_member;

	AccessExpression()
	    : Expr {ASTTag::AccessExpression} {}
};

struct TernaryExpression : public Expr {
	Expr* m_condition;
	Expr* m_then_expr;
	Expr* m_else_expr;

	TernaryExpression()
	    : Expr {ASTTag::TernaryExpression} {}
};

struct MatchExpression : public Expr {
	struct CaseData {
		Declaration m_declaration;
		Expr* m_expression;
	};

	Identifier m_target;
	Expr* m_type_hint {nullptr};
	std::unordered_map<InternedString, CaseData> m_cases;

	MatchExpression()
	    : Expr {ASTTag::MatchExpression} {}
};

struct ConstructorExpression : public Expr {
	Expr* m_constructor;
	std::vector<Expr*> m_args;

	ConstructorExpression()
	    : Expr {ASTTag::ConstructorExpression} {}
};

struct Block;

struct SequenceExpression : public Expr {
	Block* m_body;

	SequenceExpression()
	    : Expr {ASTTag::SequenceExpression} {}
};


struct Block : public Stmt {
	std::vector<Stmt*> m_body;

	Block()
	    : Stmt {ASTTag::Block} {}
};

struct ReturnStatement : public Stmt {
	Expr* m_value;
	SequenceExpression* m_surrounding_seq_expr;

	ReturnStatement()
	    : Stmt {ASTTag::ReturnStatement} {}
};

struct IfElseStatement : public Stmt {
	Expr* m_condition;
	Stmt* m_body;
	Stmt* m_else_body {nullptr}; // can be nullptr

	IfElseStatement()
	    : Stmt {ASTTag::IfElseStatement} {}
};

struct WhileStatement : public Stmt {
	Expr* m_condition;
	Stmt* m_body;

	WhileStatement()
	    : Stmt {ASTTag::WhileStatement} {}
};

struct ExpressionStatement : public Stmt {
	Expr* m_expression;

	ExpressionStatement(Expr* expression)
		: Stmt {ASTTag::ExpressionStatement}
		, m_expression {expression} {}
};

struct UnionExpression : public Expr {
	std::vector<InternedString> m_constructors;
	std::vector<Expr*> m_types;
	TypeFunctionId m_value;

	UnionExpression()
	    : Expr {ASTTag::UnionExpression} {}
};

struct StructExpression : public Expr {
	std::vector<InternedString> m_fields;
	std::vector<Expr*> m_types;
	TypeFunctionId m_value;

	StructExpression()
	    : Expr {ASTTag::StructExpression} {}
};

struct TypeTerm : public Expr {
	Expr* m_callee;
	std::vector<Expr*> m_args; // should these be TypeTerms?
	MonoId m_value {-1};

	TypeTerm()
	    : Expr {ASTTag::TypeTerm} {}
};

struct BuiltinTypeFunction : public Expr {
	TypeFunctionId m_value;
	// points to the ast node this one was made from
	Expr* m_syntax;

	BuiltinTypeFunction()
	    : Expr {ASTTag::BuiltinTypeFunction} {}
};

struct Constructor : public Expr {
	MonoId m_mono;
	InternedString m_id;
	// points to the ast node this one was made from
	Expr* m_syntax;

	Constructor()
	    : Expr {ASTTag::Constructor} {}
};

} // namespace AST
