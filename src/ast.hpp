#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast_type.hpp"
#include "token.hpp"

namespace AST {

struct AST {
  protected:
	ast_type m_type;

  public:
	AST() = default;
	AST(ast_type type)
	    : m_type {type} {
	}

	ast_type type() const {
		return m_type;
	}
	virtual ~AST() = default;
};

struct IntegerLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	IntegerLiteral()
	    : AST {ast_type::IntegerLiteral} {
	}
};

struct NumberLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	NumberLiteral()
	    : AST {ast_type::NumberLiteral} {
	}
};

struct StringLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	StringLiteral()
	    : AST {ast_type::StringLiteral} {
	}
};

struct BooleanLiteral : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	BooleanLiteral()
	    : AST {ast_type::BooleanLiteral} {
	}
};

struct NullLiteral : public AST {

	NullLiteral()
	    : AST {ast_type::NullLiteral} {
	}
};

struct ObjectLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	ObjectLiteral()
	    : AST {ast_type::ObjectLiteral} {
	}
};

struct ArrayLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_elements;

	ArrayLiteral()
	    : AST {ast_type::ArrayLiteral} {
	}
};

struct DictionaryLiteral : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	DictionaryLiteral()
	    : AST {ast_type::DictionaryLiteral} {
	}
};

struct FunctionLiteral : public AST {
	std::unique_ptr<AST> m_body;
	std::vector<std::unique_ptr<AST>> m_args;

	FunctionLiteral()
	    : AST {ast_type::FunctionLiteral} {
	}
};

struct DeclarationList : public AST {
	std::vector<std::unique_ptr<AST>> m_declarations;

	DeclarationList()
	    : AST {ast_type::DeclarationList} {
	}
};

struct Declaration : public AST {
	Token const* m_identifier_token;
	std::unique_ptr<AST> m_type;  // can be nullptr
	std::unique_ptr<AST> m_value; // can be nullptr

	std::string const& identifier_text() const {
		return m_identifier_token->m_text;
	}

	Declaration()
	    : AST {ast_type::Declaration} {
	}
};

struct Identifier : public AST {
	Token const* m_token;

	std::string const& text() {
		return m_token->m_text;
	}

	Identifier()
	    : AST {ast_type::Identifier} {
	}
};

struct BinaryExpression : public AST {
	Token const* m_op_token;
	std::unique_ptr<AST> m_lhs;
	std::unique_ptr<AST> m_rhs;

	BinaryExpression()
	    : AST {ast_type::BinaryExpression} {
	}
};

struct CallExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::vector<std::unique_ptr<AST>> m_args;

	CallExpression()
	    : AST {ast_type::CallExpression} {
	}
};

struct IndexExpression : public AST {
	std::unique_ptr<AST> m_callee;
	std::unique_ptr<AST> m_index;

	IndexExpression()
	    : AST {ast_type::IndexExpression} {
	}
};

struct Block : public AST {
	std::vector<std::unique_ptr<AST>> m_body;

	Block()
	    : AST {ast_type::Block} {
	}
};

struct ReturnStatement : public AST {
	std::unique_ptr<AST> m_value;

	ReturnStatement()
	    : AST {ast_type::ReturnStatement} {
	}
};

struct IfElseStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;
	std::unique_ptr<AST> m_else_body; // can be nullptr

	IfElseStatement()
	    : AST {ast_type::IfElseStatement} {
	}
};

struct ForStatement : public AST {
	std::unique_ptr<AST> m_declaration;
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_action;
	std::unique_ptr<AST> m_body;

	ForStatement()
	    : AST {ast_type::ForStatement} {
	}
};

struct WhileStatement : public AST {
	std::unique_ptr<AST> m_condition;
	std::unique_ptr<AST> m_body;

	WhileStatement()
	    : AST {ast_type::WhileStatement} {
	}
};

struct TypeTerm : public AST {
	std::unique_ptr<AST> m_callee;
	std::vector<std::unique_ptr<AST>> m_args;

	TypeTerm()
	    : AST {ast_type::TypeTerm} {
	}
};

void print(AST*, int);

} // namespace AST
