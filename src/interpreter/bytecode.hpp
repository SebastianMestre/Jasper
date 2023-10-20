#pragma once

#include <vector>
#include "../ast.hpp"
#include "../utils/writer.hpp"
#include "interpreter.hpp"

namespace Bytecode {

struct Instruction {
	enum class Tag { GetGlobal, NewInteger };

	Instruction(Tag tag)
	    : m_tag {tag} {}

	Tag tag() const { return m_tag; }
private:
	Tag m_tag;
};

struct GetGlobal : Instruction {
	GetGlobal(InternedString name)
	    : Instruction {Tag::GetGlobal}
	    , m_name {name} {}

	InternedString m_name;
};

struct NewInteger : Instruction {
	NewInteger(int value)
	    : Instruction {Tag::NewInteger}
	    , m_value {value} {}

	int m_value;
};

struct BasicBlock {
	std::vector<char> bytecode;
};

struct Executable {
	std::vector<BasicBlock> blocks;
};

Writer<Executable> compile(AST::Expr*);

void execute(Executable const&, Interpreter::Interpreter&);

} // namespace Bytecode
