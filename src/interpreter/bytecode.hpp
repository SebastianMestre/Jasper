#pragma once

#include <vector>

namespace Bytecode {

struct Instruction {
	enum class Tag { NewInteger };

	Instruction(Tag tag)
	    : m_tag {tag} {}

	Tag tag() const { return m_tag; }
private:
	Tag m_tag;
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

} // namespace Bytecode
