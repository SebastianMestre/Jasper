#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <climits>

#include "./utils/interned_string.hpp"
#include "token.hpp"
#include "typechecker_types.hpp"
#include "typed_ast_tag.hpp"

namespace AST {
struct AST;
}

namespace TypedAST {

struct TypedAST {
  protected:
	TypedASTTag m_type;

  public:
	TypedAST(TypedASTTag type)
	    : m_type {type} {}

	MetaTypeId m_meta_type {-1};
	// is not set on polymorphic declarations and typefuncs (TODO: refactor)
	MonoId m_value_type {-1};
	TypedASTTag type() const {
		return m_type;
	}
	virtual ~TypedAST() = default;
};

struct Allocator;

TypedAST* convert_ast(AST::AST*, Allocator& alloc);

struct FunctionLiteral;

struct Declaration : public TypedAST {
	InternedString m_identifier;
	Token const* m_identifier_token {nullptr}; // used for error reports only. Is nullptr if the declaration doesn't come from parsing a source file

	TypedAST* m_type_hint {nullptr};  // can be nullptr
	TypedAST* m_value {nullptr}; // can be nullptr

	std::unordered_set<Declaration*> m_references;

	bool m_is_polymorphic {false};
	PolyId m_decl_type;

	int m_frame_offset {INT_MIN};

	// nullptr means global
	FunctionLiteral* m_surrounding_function {nullptr};

	InternedString const& identifier_text() const;

	Declaration()
	    : TypedAST {TypedASTTag::Declaration} {}
};

// doesnt have a ast_vtype
struct DeclarationList : public TypedAST {
	std::vector<Declaration> m_declarations;

	DeclarationList()
	    : TypedAST {TypedASTTag::DeclarationList} {}
};

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct NumberLiteral : public TypedAST {
	float m_value;
	Token const* m_token;

	float value() const {
		return m_value;
	}

	std::string const& text() {
		return m_token->m_text.str();
	}

	NumberLiteral()
	    : TypedAST {TypedASTTag::NumberLiteral} {}
};

struct IntegerLiteral : public TypedAST {
	int m_value;
	Token const* m_token;

	int value() const {
		return m_value;
	}

	std::string const& text() {
		return m_token->m_text.str();
	}

	IntegerLiteral()
	    : TypedAST {TypedASTTag::IntegerLiteral} {}
};

struct StringLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	StringLiteral()
	    : TypedAST {TypedASTTag::StringLiteral} {}
};

struct BooleanLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	BooleanLiteral()
	    : TypedAST {TypedASTTag::BooleanLiteral} {}
};

struct NullLiteral : public TypedAST {

	NullLiteral()
	    : TypedAST {TypedASTTag::NullLiteral} {}
};

struct ArrayLiteral : public TypedAST {
	std::vector<TypedAST*> m_elements;

	ArrayLiteral()
	    : TypedAST {TypedASTTag::ArrayLiteral} {}
};

struct DictionaryLiteral : public TypedAST {
	std::vector<Declaration> m_body;

	DictionaryLiteral()
	    : TypedAST {TypedASTTag::DictionaryLiteral} {}
};

struct FunctionLiteral : public TypedAST {
	struct CaptureData {
		Declaration* outer_declaration{nullptr};
		int outer_frame_offset{INT_MIN};
		int inner_frame_offset{INT_MIN};
	};

	MonoId m_return_type;
	TypedAST* m_body;
	std::vector<Declaration> m_args;
	std::unordered_map<InternedString, CaptureData> m_captures;
	FunctionLiteral* m_surrounding_function {nullptr};

	FunctionLiteral()
	    : TypedAST {TypedASTTag::FunctionLiteral} {}
};

// the ast_vtype must be computed
struct Identifier : public TypedAST {
	enum class Origin { Global, Capture, Local };

	Token const* m_token;
	Declaration* m_declaration {nullptr}; // can be nullptr
	FunctionLiteral* m_surrounding_function {nullptr};

	Origin m_origin;
	int m_frame_offset {INT_MIN};

	InternedString const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : TypedAST {TypedASTTag::Identifier} {}
};

// the value depends on the return value of callee
struct CallExpression : public TypedAST {
	TypedAST* m_callee;
	std::vector<TypedAST*> m_args;

	CallExpression()
	    : TypedAST {TypedASTTag::CallExpression} {}
};

struct IndexExpression : public TypedAST {
	TypedAST* m_callee;
	TypedAST* m_index;

	IndexExpression()
	    : TypedAST {TypedASTTag::IndexExpression} {}
};

struct AccessExpression : public TypedAST {
	TypedAST* m_record;
	Token const* m_member;

	AccessExpression()
	    : TypedAST {TypedASTTag::AccessExpression} {}
};

struct TernaryExpression : public TypedAST {
	TypedAST* m_condition;
	TypedAST* m_then_expr;
	TypedAST* m_else_expr;

	TernaryExpression()
	    : TypedAST {TypedASTTag::TernaryExpression} {}
};

struct MatchExpression : public TypedAST {
	struct CaseData {
		Declaration m_declaration;
		TypedAST* m_expression;
	};

	Identifier m_matchee;
	TypedAST* m_type_hint {nullptr};
	std::unordered_map<InternedString, CaseData> m_cases;

	MatchExpression()
	    : TypedAST {TypedASTTag::MatchExpression} {}
};

struct ConstructorExpression : public TypedAST {
	TypedAST* m_constructor;
	std::vector<TypedAST*> m_args;

	ConstructorExpression()
	    : TypedAST {TypedASTTag::ConstructorExpression} {}
};

struct Block;

struct SequenceExpression : public TypedAST {
	Block* m_body;

	SequenceExpression()
	    : TypedAST {TypedASTTag::SequenceExpression} {}
};

struct Block : public TypedAST {
	std::vector<TypedAST*> m_body;

	Block()
	    : TypedAST {TypedASTTag::Block} {}
};

struct ReturnStatement : public TypedAST {
	TypedAST* m_value;

	ReturnStatement()
	    : TypedAST {TypedASTTag::ReturnStatement} {}
};

struct IfElseStatement : public TypedAST {
	TypedAST* m_condition;
	TypedAST* m_body;
	TypedAST* m_else_body {nullptr}; // can be nullptr

	IfElseStatement()
	    : TypedAST {TypedASTTag::IfElseStatement} {}
};

struct ForStatement : public TypedAST {
	Declaration m_declaration;
	TypedAST* m_condition;
	TypedAST* m_action;
	TypedAST* m_body;

	ForStatement()
	    : TypedAST {TypedASTTag::ForStatement} {}
};

struct WhileStatement : public TypedAST {
	TypedAST* m_condition;
	TypedAST* m_body;

	WhileStatement()
	    : TypedAST {TypedASTTag::WhileStatement} {}
};

struct UnionExpression : public TypedAST {
	std::vector<Identifier> m_constructors;
	std::vector<TypedAST*> m_types;

	UnionExpression()
	    : TypedAST {TypedASTTag::UnionExpression} {}
};

struct StructExpression : public TypedAST {
	std::vector<Identifier> m_fields;
	std::vector<TypedAST*> m_types;

	StructExpression()
	    : TypedAST {TypedASTTag::StructExpression} {}
};

struct TypeTerm : public TypedAST {
	TypedAST* m_callee;
	std::vector<TypedAST*> m_args; // should these be TypeTerms?

	TypeTerm()
	    : TypedAST {TypedASTTag::TypeTerm} {}
};

struct TypeFunctionHandle : public TypedAST {
	TypeFunctionId m_value;
	// points to the ast node this one was made from
	TypedAST* m_syntax;

	TypeFunctionHandle()
	    : TypedAST {TypedASTTag::TypeFunctionHandle} {}
};

struct MonoTypeHandle : public TypedAST {
	MonoId m_value;
	// points to the ast node this one was made from
	TypedAST* m_syntax;

	MonoTypeHandle()
	    : TypedAST {TypedASTTag::MonoTypeHandle} {}
};

struct Constructor : public TypedAST {
	MonoId m_mono;
	Token const* m_id;
	// points to the ast node this one was made from
	TypedAST* m_syntax;

	Constructor()
	    : TypedAST {TypedASTTag::Constructor} {}
};

} // namespace TypedAST
