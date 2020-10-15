#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <climits>

#include "token.hpp"
#include "token_tag.hpp"
#include "typed_ast_tag.hpp"
#include "typedefs.hpp"
#include "typechecker_types.hpp"

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
	Token const* m_identifier_token;
	TypedAST* m_value {nullptr}; // can be nullptr

	std::unordered_set<Declaration*> m_references;

	bool m_is_polymorphic {false};
	PolyId m_decl_type;

	int m_frame_offset {INT_MIN};

	// nullptr means global
	FunctionLiteral* m_surrounding_function {nullptr};

	InternedString const& identifier_text() const {
		return m_identifier_token->m_text;
	}

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

struct ObjectLiteral : public TypedAST {
	std::vector<Declaration> m_body;

	// future feature
	// the value type for objects must be followeb by a class identifier
	ObjectLiteral()
	    : TypedAST {TypedASTTag::ObjectLiteral} {}
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

struct RecordAccessExpression : public TypedAST {
	TypedAST* m_record;
	Token const* m_member;

	RecordAccessExpression()
	    : TypedAST {TypedASTTag::RecordAccessExpression} {}
};

struct TernaryExpression : public TypedAST {
	TypedAST* m_condition;
	TypedAST* m_then_expr;
	TypedAST* m_else_expr;

	TernaryExpression()
	    : TypedAST {TypedASTTag::TernaryExpression} {}
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

} // namespace TypedAST
