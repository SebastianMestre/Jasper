#pragma once

#include <memory>
#include <string>
#include <vector>

namespace RTAST {

enum class rtast_type {
	FunctionCall,
	ConstructorCall,
	Sequence,

	NumberLiteral,
	StringLiteral,
	FunctionLiteral,

	LetBlock, // TODO: is this really needed?
	RecBlock,
};

struct RTAST {
	rtast_type m_type;

	// Everything is an expression, so everything has a monotype
	int m_monotype;
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
};

struct LetBlock final : public RTAST {
	PolymorphicDeclaration m_declaration_block;
	std::unique_ptr<RTAST> m_in_block;
};

struct FunctionCall final : public RTAST {
	std::unique_ptr<RTAST> m_callee;
};

struct ConstructorCall final : public RTAST {
	// TODO: what if we are not given a fully instanced type?
	int m_monotype_callee;

	std::vector<std::unique_ptr<RTAST>> m_callee;
};

struct RealLiteral final : public RTAST {
	double m_value;
};

struct IntegerLiteral final : public RTAST {
	int m_value;
};

struct StringLiteral final : public RTAST {
	std::string m_value;
};

struct FunctionLiteral final : public RTAST {
	std::vector<MonomorphicDeclaration> m_arguments;
	std::unique_ptr<RTAST> m_body;
};

struct Sequence final : public RTAST {
	std::vector<std::unique_ptr<RTAST>> m_values;
};

}
