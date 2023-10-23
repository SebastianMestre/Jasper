#pragma once

#include <vector>
#include "../ast.hpp"
#include "../utils/writer.hpp"
#include "interpreter.hpp"

namespace Bytecode {

struct Instruction {
	enum class Tag { GetGlobal, NewInteger, Call };

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

struct Call : Instruction {
	Call(int argument_count)
	    : Instruction {Tag::Call}
	    , m_argument_count {argument_count} {}

	int m_argument_count;
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
