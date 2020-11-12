#pragma once

namespace TypedAST {
struct TypedAST;
}

/* this namespace name is not very good */
namespace TypeChecker {

enum class ValueTag {
	RuntimeExpression,
	TypeFunctionHandle,
	MonoTypeHandle,
};

struct Value {
	ValueTag m_tag;
	ValueTag tag() const {
		return m_tag;
	}
};

struct RuntimeExpression : public Value {
	TypedAST::TypedAST* m_ast;
	RuntimeExpression()
	    : Value {ValueTag::RuntimeExpression} {}
};

struct TypeFunctionHandle : public Value {
	int m_id;
	TypeFunctionHandle()
	    : Value {ValueTag::TypeFunctionHandle} {}
};

struct MonoTypeHandle : public Value {
	int m_id;
	MonoTypeHandle()
	    : Value {ValueTag::MonoTypeHandle} {}
};

}
