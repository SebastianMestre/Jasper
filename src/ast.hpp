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

struct Allocator;

AST* convert_ast(CST::CST*, Allocator& alloc);

struct FunctionLiteral;
struct SequenceExpression;

struct Declaration : public AST {
	InternedString m_identifier;

	AST* m_type_hint {nullptr};  // can be nullptr
	AST* m_value {nullptr}; // can be nullptr

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
struct DeclarationList : public AST {
	std::vector<Declaration> m_declarations;

	DeclarationList()
	    : AST {ASTTag::DeclarationList} {}
};

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct NumberLiteral : public AST {
	float m_value;

	float value() const {
		return m_value;
	}

	NumberLiteral()
	    : AST {ASTTag::NumberLiteral} {}
};

struct IntegerLiteral : public AST {
	int m_value;

	int value() const {
		return m_value;
	}

	IntegerLiteral()
	    : AST {ASTTag::IntegerLiteral} {}
};

struct StringLiteral : public AST {
	InternedString m_text;

	std::string const& text() {
		return m_text.str();
	}

	StringLiteral()
	    : AST {ASTTag::StringLiteral} {}
};

struct BooleanLiteral : public AST {
	bool m_value;

	BooleanLiteral()
	    : AST {ASTTag::BooleanLiteral} {}
};

struct NullLiteral : public AST {

	NullLiteral()
	    : AST {ASTTag::NullLiteral} {}
};

struct ArrayLiteral : public AST {
	std::vector<AST*> m_elements;

	ArrayLiteral()
	    : AST {ASTTag::ArrayLiteral} {}
};

struct FunctionLiteral : public AST {
	struct CaptureData {
		Declaration* outer_declaration{nullptr};
		int outer_frame_offset{INT_MIN};
		int inner_frame_offset{INT_MIN};
	};

	MonoId m_return_type;
	AST* m_body;
	std::vector<Declaration> m_args;
	std::unordered_map<InternedString, CaptureData> m_captures;
	FunctionLiteral* m_surrounding_function {nullptr};

	FunctionLiteral()
	    : AST {ASTTag::FunctionLiteral} {}
};

// the ast_vtype must be computed
struct Identifier : public AST {
	enum class Origin { Global, Capture, Local };

	InternedString m_text;
	Declaration* m_declaration {nullptr}; // can be nullptr
	FunctionLiteral* m_surrounding_function {nullptr};

	Origin m_origin;
	int m_frame_offset {INT_MIN};

	Token const* token() const;
	InternedString const& text() const {
		return m_text;
	}

	Identifier()
	    : AST {ASTTag::Identifier} {}
};

// the value depends on the return value of callee
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
	AST* m_record;
	InternedString m_member;

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
	struct CaseData {
		Declaration m_declaration;
		AST* m_expression;
	};

	Identifier m_matchee;
	AST* m_type_hint {nullptr};
	std::unordered_map<InternedString, CaseData> m_cases;

	MatchExpression()
	    : AST {ASTTag::MatchExpression} {}
};

struct ConstructorExpression : public AST {
	AST* m_constructor;
	std::vector<AST*> m_args;

	ConstructorExpression()
	    : AST {ASTTag::ConstructorExpression} {}
};

struct Block;

struct SequenceExpression : public AST {
	Block* m_body;

	SequenceExpression()
	    : AST {ASTTag::SequenceExpression} {}
};

struct Block : public AST {
	std::vector<AST*> m_body;

	Block()
	    : AST {ASTTag::Block} {}
};

struct ReturnStatement : public AST {
	AST* m_value;
	SequenceExpression* m_surrounding_seq_expr;

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

struct UnionExpression : public AST {
	std::vector<Identifier> m_constructors;
	std::vector<AST*> m_types;

	UnionExpression()
	    : AST {ASTTag::UnionExpression} {}
};

struct StructExpression : public AST {
	std::vector<Identifier> m_fields;
	std::vector<AST*> m_types;

	StructExpression()
	    : AST {ASTTag::StructExpression} {}
};

struct TypeTerm : public AST {
	AST* m_callee;
	std::vector<AST*> m_args; // should these be TypeTerms?

	TypeTerm()
	    : AST {ASTTag::TypeTerm} {}
};

struct TypeFunctionHandle : public AST {
	TypeFunctionId m_value;
	// points to the ast node this one was made from
	AST* m_syntax;

	TypeFunctionHandle()
	    : AST {ASTTag::TypeFunctionHandle} {}
};

struct MonoTypeHandle : public AST {
	MonoId m_value;
	// points to the ast node this one was made from
	AST* m_syntax;

	MonoTypeHandle()
	    : AST {ASTTag::MonoTypeHandle} {}
};

struct Constructor : public AST {
	MonoId m_mono;
	InternedString m_id;
	// points to the ast node this one was made from
	AST* m_syntax;

	Constructor()
	    : AST {ASTTag::Constructor} {}
};

} // namespace AST
