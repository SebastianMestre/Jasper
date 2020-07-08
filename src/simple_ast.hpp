#pragma once

#include <memory>
#include <string>
#include <vector>

namespace RTAST {

enum class rtast_type {
	FunctionCall,
	ConstructorCall,
	Sequence,
	Return,

	IntegerLiteral,
	RealLiteral,
	StringLiteral,
	FunctionLiteral,

	LetBlock, // TODO: is this really needed?
	RecBlock,
};

struct RTAST {
	rtast_type m_type;

	// Everything is an expression, so everything has a monotype
	int m_monotype;

	RTAST(rtast_type type) : m_type { type } {}
};

struct MonomorphicDeclaration {
	std::string m_name;
	std::unique_ptr<RTAST> m_value;
	int m_monotype;
};

struct PolymorphicDeclaration {
	std::string m_name;
	std::unique_ptr<RTAST> m_value;
	int m_polytype;
};

struct RecBlock final : public RTAST {
	std::vector<MonomorphicDeclaration> m_declaration_block;
	std::unique_ptr<RTAST> m_in_block;
	RecBlock() : RTAST { rtast_type::RecBlock } {}
};

struct LetBlock final : public RTAST {
	PolymorphicDeclaration m_declaration_block;
	std::unique_ptr<RTAST> m_in_block;
	LetBlock() : RTAST { rtast_type::LetBlock } {}
};

struct FunctionCall final : public RTAST {
	std::unique_ptr<RTAST> m_callee;
	FunctionCall() : RTAST { rtast_type::FunctionCall } {}
};

struct ConstructorCall final : public RTAST {
	// TODO: what if we are not given a fully instanced type?
	int m_monotype_callee;

	std::vector<std::unique_ptr<RTAST>> m_callee;
	ConstructorCall() : RTAST { rtast_type::ConstructorCall } {}
};

struct RealLiteral final : public RTAST {
	double m_value;
	RealLiteral() : RTAST { rtast_type::RealLiteral } {}
};

struct IntegerLiteral final : public RTAST {
	int m_value;
	IntegerLiteral() : RTAST { rtast_type::IntegerLiteral } {}
};

struct StringLiteral final : public RTAST {
	std::string m_value;
	StringLiteral() : RTAST { rtast_type::StringLiteral } {}
};

struct FunctionLiteral final : public RTAST {
	std::vector<MonomorphicDeclaration> m_arguments;
	std::unique_ptr<RTAST> m_body;
	FunctionLiteral() : RTAST { rtast_type::FunctionLiteral } {}
};

struct Sequence final : public RTAST {
	std::vector<std::unique_ptr<RTAST>> m_values;
	Sequence() : RTAST { rtast_type::Sequence } {}
};

struct Return final : public RTAST {
	std::unique_ptr<RTAST> m_value;
	Return() : RTAST { rtast_type::Return } {}
};

}
