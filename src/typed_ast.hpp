#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

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

Own<TypedAST> convert_ast(AST::AST*);

struct FunctionLiteral;

struct Declaration : public TypedAST {
	Token const* m_identifier_token;
	Own<TypedAST> m_value; // can be nullptr

	std::unordered_set<Declaration*> m_references;

	bool m_is_polymorphic {false};
	PolyId m_decl_type;

	int m_frame_offset {-1};

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
	std::vector<Own<Declaration>> m_declarations;

	DeclarationList()
	    : TypedAST {TypedASTTag::DeclarationList} {}
};

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct NumberLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text.str();
	}

	NumberLiteral()
	    : TypedAST {TypedASTTag::NumberLiteral} {}
};

struct IntegerLiteral : public TypedAST {
	Token const* m_token;

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
	std::vector<Own<TypedAST>> m_body;

	// future feature
	// the value type for objects must be followeb by a class identifier
	ObjectLiteral()
	    : TypedAST {TypedASTTag::ObjectLiteral} {}
};

struct ArrayLiteral : public TypedAST {
	std::vector<Own<TypedAST>> m_elements;

	ArrayLiteral()
	    : TypedAST {TypedASTTag::ArrayLiteral} {}
};

struct DictionaryLiteral : public TypedAST {
	std::vector<Own<TypedAST>> m_body;

	DictionaryLiteral()
	    : TypedAST {TypedASTTag::DictionaryLiteral} {}
};

struct FunctionLiteral : public TypedAST {
	MonoId m_return_type;
	Own<TypedAST> m_body;
	std::vector<Declaration> m_args;
	std::unordered_set<InternedString> m_captures;

	FunctionLiteral()
	    : TypedAST {TypedASTTag::FunctionLiteral} {}
};

// the ast_vtype must be computed
struct Identifier : public TypedAST {
	enum class Origin { Global, Capture, Local };

	Token const* m_token;
	Declaration* m_declaration {nullptr};

	Origin m_origin;
	int m_frame_offset {-1};

	InternedString const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : TypedAST {TypedASTTag::Identifier} {}
};

// the value depends on the return value of callee
struct CallExpression : public TypedAST {
	Own<TypedAST> m_callee;
	std::vector<Own<TypedAST>> m_args;

	CallExpression()
	    : TypedAST {TypedASTTag::CallExpression} {}
};

struct IndexExpression : public TypedAST {
	Own<TypedAST> m_callee;
	Own<TypedAST> m_index;

	IndexExpression()
	    : TypedAST {TypedASTTag::IndexExpression} {}
};

struct RecordAccessExpression : public TypedAST {
	Own<TypedAST> m_record;
	Token const* m_member;

	RecordAccessExpression()
	    : TypedAST {TypedASTTag::RecordAccessExpression} {}
};

struct TernaryExpression : public TypedAST {
	Own<TypedAST> m_condition;
	Own<TypedAST> m_then_expr;
	Own<TypedAST> m_else_expr;

	TernaryExpression()
	    : TypedAST {TypedASTTag::TernaryExpression} {}
};

struct Block : public TypedAST {
	std::vector<Own<TypedAST>> m_body;

	Block()
	    : TypedAST {TypedASTTag::Block} {}
};

struct ReturnStatement : public TypedAST {
	Own<TypedAST> m_value;

	ReturnStatement()
	    : TypedAST {TypedASTTag::ReturnStatement} {}
};

struct IfElseStatement : public TypedAST {
	Own<TypedAST> m_condition;
	Own<TypedAST> m_body;
	Own<TypedAST> m_else_body; // can be nullptr

	IfElseStatement()
	    : TypedAST {TypedASTTag::IfElseStatement} {}
};

struct ForStatement : public TypedAST {
	Own<TypedAST> m_declaration;
	Own<TypedAST> m_condition;
	Own<TypedAST> m_action;
	Own<TypedAST> m_body;

	ForStatement()
	    : TypedAST {TypedASTTag::ForStatement} {}
};

struct WhileStatement : public TypedAST {
	Own<TypedAST> m_condition;
	Own<TypedAST> m_body;

	WhileStatement()
	    : TypedAST {TypedASTTag::WhileStatement} {}
};

struct StructExpression : public TypedAST {
	std::vector<Identifier> m_fields;
	std::vector<Own<TypedAST>> m_types;

	StructExpression()
	    : TypedAST {TypedASTTag::StructExpression} {}
};

struct TypeTerm : public TypedAST {
	Own<TypedAST> m_callee;
	std::vector<Own<TypedAST>> m_args; // should these be TypeTerms?

	TypeTerm()
	    : TypedAST {TypedASTTag::TypeTerm} {}
};

struct TypeFunctionHandle : public TypedAST {
	TypeFunctionId m_value;
	// points to the ast node this one was made from
	Own<TypedAST> m_syntax;

	TypeFunctionHandle()
	    : TypedAST {TypedASTTag::TypeFunctionHandle} {}
};

struct MonoTypeHandle : public TypedAST {
	MonoId m_value;
	// points to the ast node this one was made from
	Own<TypedAST> m_syntax;

	MonoTypeHandle()
	    : TypedAST {TypedASTTag::MonoTypeHandle} {}
};

} // namespace TypedAST
