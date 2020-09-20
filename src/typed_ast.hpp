#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "ast_tag.hpp"
#include "token.hpp"
#include "token_tag.hpp"
#include "typedefs.hpp"
#include "typesystem_types.hpp"

namespace AST {
struct AST;
}

namespace TypedAST {

struct TypedAST {
  protected:
	ASTTag m_type;

  public:
	TypedAST(ASTTag type)
	    : m_type {type} {}

	// is not set on polymorphic declarations
	MonoId m_value_type{-1};
	ASTTag type() const {
		return m_type;
	}
	virtual ~TypedAST() = default;
};

TypedAST* convertAST(AST::AST*);
Own<TypedAST> get_unique(Own<AST::AST>&);

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct NumberLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	NumberLiteral()
	    : TypedAST {ASTTag::NumberLiteral} {}
};

struct IntegerLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	IntegerLiteral()
	    : TypedAST {ASTTag::IntegerLiteral} {}
};

struct StringLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	StringLiteral()
	    : TypedAST {ASTTag::StringLiteral} {}
};

struct BooleanLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	BooleanLiteral()
	    : TypedAST {ASTTag::BooleanLiteral} {}
};

struct NullLiteral : public TypedAST {

	NullLiteral()
	    : TypedAST {ASTTag::NullLiteral} {}
};

struct ObjectLiteral : public TypedAST {
	std::vector<Own<TypedAST>> m_body;

	// future feature
	// the value type for objects must be followeb by a class identifier
	ObjectLiteral()
	    : TypedAST {ASTTag::ObjectLiteral} {}
};

struct ArrayLiteral : public TypedAST {
	std::vector<Own<TypedAST>> m_elements;

	ArrayLiteral()
	    : TypedAST {ASTTag::ArrayLiteral} {}
};

struct DictionaryLiteral : public TypedAST {
	std::vector<Own<TypedAST>> m_body;

	DictionaryLiteral()
	    : TypedAST {ASTTag::DictionaryLiteral} {}
};

struct FunctionArgument {
	Token const* m_identifier_token;
	MonoId m_value_type;

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}
};

struct FunctionLiteral : public TypedAST {
	MonoId m_return_type;
	Own<TypedAST> m_body;
	std::vector<FunctionArgument> m_args;
	std::unordered_set<std::string> m_captures;

	FunctionLiteral()
	    : TypedAST {ASTTag::FunctionLiteral} {}
};

// doesnt have a ast_vtype
struct DeclarationList : public TypedAST {
	std::vector<Own<TypedAST>> m_declarations;

	DeclarationList()
	    : TypedAST {ASTTag::DeclarationList} {}
};

struct Declaration : public TypedAST {
	Token const* m_identifier_token;
	Own<TypedAST> m_value; // can be nullptr
	std::unordered_set<Declaration*> m_references;
	bool m_is_polymorphic {false};
	PolyId m_decl_type;

	// nullptr means global
	FunctionLiteral* m_surrounding_function {nullptr};

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}

	Declaration()
	    : TypedAST {ASTTag::Declaration} {}
};

// the ast_vtype must be computed
struct Identifier : public TypedAST {
	Token const* m_token;
	Declaration* m_declaration {nullptr};

	std::string const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : TypedAST {ASTTag::Identifier} {}
};

// the value depends on the return value of callee
struct CallExpression : public TypedAST {
	Own<TypedAST> m_callee;
	std::vector<Own<TypedAST>> m_args;

	CallExpression()
	    : TypedAST {ASTTag::CallExpression} {}
};

struct IndexExpression : public TypedAST {
	Own<TypedAST> m_callee;
	Own<TypedAST> m_index;

	IndexExpression()
	    : TypedAST {ASTTag::IndexExpression} {}
};

struct RecordAccessExpression : public TypedAST {
	Own<TypedAST> m_record;
	Own<Identifier> m_member;

	RecordAccessExpression()
	    : TypedAST {ASTTag::RecordAccessExpression} {}
};

struct TernaryExpression : public TypedAST {
	Own<TypedAST> m_condition;
	Own<TypedAST> m_then_expr;
	Own<TypedAST> m_else_expr;

	TernaryExpression()
	    : TypedAST {ASTTag::TernaryExpression} {}
};

struct Block : public TypedAST {
	std::vector<Own<TypedAST>> m_body;

	Block()
	    : TypedAST {ASTTag::Block} {}
};

struct ReturnStatement : public TypedAST {
	Own<TypedAST> m_value;

	ReturnStatement()
	    : TypedAST {ASTTag::ReturnStatement} {}
};

struct IfElseStatement : public TypedAST {
	Own<TypedAST> m_condition;
	Own<TypedAST> m_body;
	Own<TypedAST> m_else_body; // can be nullptr

	IfElseStatement()
	    : TypedAST {ASTTag::IfElseStatement} {}
};

struct ForStatement : public TypedAST {
	Own<TypedAST> m_declaration;
	Own<TypedAST> m_condition;
	Own<TypedAST> m_action;
	Own<TypedAST> m_body;

	ForStatement()
	    : TypedAST {ASTTag::ForStatement} {}
};

struct WhileStatement : public TypedAST {
	Own<TypedAST> m_condition;
	Own<TypedAST> m_body;

	WhileStatement()
	    : TypedAST {ASTTag::WhileStatement} {}
};

} // namespace TypedAST
