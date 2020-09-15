#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "ast_type.hpp"
#include "token.hpp"
#include "token_type.hpp"
#include "typesystem_types.hpp"

namespace AST {
struct AST;
}

namespace TypedAST {

struct TypedAST {
  protected:
	ASTType m_type;

  public:
	TypedAST(ASTType type)
	    : m_type {type} {}

	// is not set on polymorphic declarations
	MonoId m_value_type{-1};
	ASTType type() const {
		return m_type;
	}
	virtual ~TypedAST() = default;
};

TypedAST* convertAST(AST::AST*);
std::unique_ptr<TypedAST> get_unique(std::unique_ptr<AST::AST>&);

// las estructuras como declaration list, index expression, block, if, for no tienen
// tipo de valor asociado
struct NumberLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	NumberLiteral()
	    : TypedAST {ASTType::NumberLiteral} {}
};

struct IntegerLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	IntegerLiteral()
	    : TypedAST {ASTType::IntegerLiteral} {}
};

struct StringLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	StringLiteral()
	    : TypedAST {ASTType::StringLiteral} {}
};

struct BooleanLiteral : public TypedAST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	BooleanLiteral()
	    : TypedAST {ASTType::BooleanLiteral} {}
};

struct NullLiteral : public TypedAST {

	NullLiteral()
	    : TypedAST {ASTType::NullLiteral} {}
};

struct ObjectLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	// future feature
	// the value type for objects must be followeb by a class identifier
	ObjectLiteral()
	    : TypedAST {ASTType::ObjectLiteral} {}
};

struct ArrayLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_elements;

	ArrayLiteral()
	    : TypedAST {ASTType::ArrayLiteral} {}
};

struct DictionaryLiteral : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	DictionaryLiteral()
	    : TypedAST {ASTType::DictionaryLiteral} {}
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
	std::unique_ptr<TypedAST> m_body;
	std::vector<FunctionArgument> m_args;
	std::unordered_set<std::string> m_captures;

	FunctionLiteral()
	    : TypedAST {ASTType::FunctionLiteral} {}
};

// doesnt have a ast_vtype
struct DeclarationList : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_declarations;

	DeclarationList()
	    : TypedAST {ASTType::DeclarationList} {}
};

struct Declaration : public TypedAST {
	Token const* m_identifier_token;
	std::unique_ptr<TypedAST> m_value; // can be nullptr
	std::unordered_set<Declaration*> m_references;
	bool m_is_polymorphic {false};
	PolyId m_decl_type;

	// nullptr means global
	FunctionLiteral* m_surrounding_function {nullptr};

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}

	Declaration()
	    : TypedAST {ASTType::Declaration} {}
};

// the ast_vtype must be computed
struct Identifier : public TypedAST {
	Token const* m_token;
	Declaration* m_declaration {nullptr};

	std::string const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : TypedAST {ASTType::Identifier} {}
};

// the value depends on the return value of callee
struct CallExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_callee;
	std::vector<std::unique_ptr<TypedAST>> m_args;

	CallExpression()
	    : TypedAST {ASTType::CallExpression} {}
};

struct IndexExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_callee;
	std::unique_ptr<TypedAST> m_index;

	IndexExpression()
	    : TypedAST {ASTType::IndexExpression} {}
};

struct TernaryExpression : public TypedAST {
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_then_expr;
	std::unique_ptr<TypedAST> m_else_expr;

	TernaryExpression()
	    : TypedAST {ASTType::TernaryExpression} {}
};

struct Block : public TypedAST {
	std::vector<std::unique_ptr<TypedAST>> m_body;

	Block()
	    : TypedAST {ASTType::Block} {}
};

struct ReturnStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_value;

	ReturnStatement()
	    : TypedAST {ASTType::ReturnStatement} {}
};

struct IfElseStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_body;
	std::unique_ptr<TypedAST> m_else_body; // can be nullptr

	IfElseStatement()
	    : TypedAST {ASTType::IfElseStatement} {}
};

struct ForStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_declaration;
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_action;
	std::unique_ptr<TypedAST> m_body;

	ForStatement()
	    : TypedAST {ASTType::ForStatement} {}
};

struct WhileStatement : public TypedAST {
	std::unique_ptr<TypedAST> m_condition;
	std::unique_ptr<TypedAST> m_body;

	WhileStatement()
	    : TypedAST {ASTType::WhileStatement} {}
};

} // namespace TypedAST
