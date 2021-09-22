#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <climits>

#include "./utils/interned_string.hpp"
#include "typechecker_types.hpp"
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
	MetaTypeId m_meta_type {-1};
	// is not set on polymorphic declarations and typefuncs (TODO: refactor)
	MonoId m_value_type {-1};
	ASTTag type() const {
		return m_type;
	}
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

};

struct Allocator;

AST* convert_ast(CST::CST*, Allocator& alloc);

struct FunctionLiteral;
struct SequenceExpression;

struct Declaration : public AST {
	InternedString m_identifier;

	Expr* m_type_hint {nullptr};  // can be nullptr
	Expr* m_value {nullptr}; // can be nullptr

	std::unordered_set<Declaration*> m_references;

	bool m_is_polymorphic {false};
	PolyId m_decl_type;

	int m_frame_offset {INT_MIN};

	FunctionLiteral* m_surrounding_function {nullptr};
	SequenceExpression* m_surrounding_seq_expr {nullptr};

	bool is_global() const {
		return !m_surrounding_function && !m_surrounding_seq_expr;
	}

	InternedString const& identifier_text() const;

	Declaration()
	    : AST {ASTTag::Declaration} {}
};

// doesnt have a ast_vtype
struct Program : public AST {
	std::vector<Declaration> m_declarations;

	Program()
	    : AST {ASTTag::Program} {}
};

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
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

	MonoId m_return_type;
	Expr* m_body;
	std::vector<Declaration> m_args;
	std::unordered_map<InternedString, CaptureData> m_captures;
	FunctionLiteral* m_surrounding_function {nullptr};

	FunctionLiteral()
	    : Expr {ASTTag::FunctionLiteral} {}
};

// the ast_vtype must be computed
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

// the value depends on the return value of callee
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
	AST* m_type_hint {nullptr};
	std::unordered_map<InternedString, CaseData> m_cases;

	MatchExpression()
	    : Expr {ASTTag::MatchExpression} {}
};

struct ConstructorExpression : public Expr {
	AST* m_constructor;
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

struct Block : public AST {
	std::vector<AST*> m_body;

	Block()
	    : AST {ASTTag::Block} {}
};

struct ReturnStatement : public AST {
	Expr* m_value;
	SequenceExpression* m_surrounding_seq_expr;

	ReturnStatement()
	    : AST {ASTTag::ReturnStatement} {}
};

struct IfElseStatement : public AST {
	Expr* m_condition;
	AST* m_body;
	AST* m_else_body {nullptr}; // can be nullptr

	IfElseStatement()
	    : AST {ASTTag::IfElseStatement} {}
};

struct WhileStatement : public AST {
	Expr* m_condition;
	AST* m_body;

	WhileStatement()
	    : AST {ASTTag::WhileStatement} {}
};

struct UnionExpression : public Expr {
	std::vector<InternedString> m_constructors;
	std::vector<AST*> m_types;

	UnionExpression()
	    : Expr {ASTTag::UnionExpression} {}
};

struct StructExpression : public Expr {
	std::vector<InternedString> m_fields;
	std::vector<AST*> m_types;

	StructExpression()
	    : Expr {ASTTag::StructExpression} {}
};

struct TypeTerm : public Expr {
	AST* m_callee;
	std::vector<AST*> m_args; // should these be TypeTerms?

	TypeTerm()
	    : Expr {ASTTag::TypeTerm} {}
};

struct TypeFunctionHandle : public Expr {
	TypeFunctionId m_value;
	// points to the ast node this one was made from
	AST* m_syntax;

	TypeFunctionHandle()
	    : Expr {ASTTag::TypeFunctionHandle} {}
};

struct MonoTypeHandle : public Expr {
	MonoId m_value;
	// points to the ast node this one was made from
	AST* m_syntax;

	MonoTypeHandle()
	    : Expr {ASTTag::MonoTypeHandle} {}
};

struct Constructor : public Expr {
	MonoId m_mono;
	InternedString m_id;
	// points to the ast node this one was made from
	AST* m_syntax;

	Constructor()
	    : Expr {ASTTag::Constructor} {}
};

} // namespace AST
