#pragma once

#include <vector>
#include "../ast.hpp"
#include "../utils/writer.hpp"
#include "interpreter.hpp"

namespace Bytecode {

struct Instruction {
	enum class Tag {
		GetGlobal,
		GetLocal,
		SetLocal,
		PushVariable,
		NewInteger,
		NewBoolean,
		NewNumber,
		NewNull,
		NewArray,
		IndexAccess,
		Call,
		Jump,
		JumpIfFalse,
		Pop,
		SaveReturn,
		FetchReturn,
		StartRegion,
		EndRegion,
	};

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

struct GetLocal : Instruction {
	GetLocal(int frame_offset)
	    : Instruction {Tag::GetLocal}
	    , m_frame_offset {frame_offset} {}

	int m_frame_offset;
};

struct SetLocal : Instruction {
	SetLocal(int frame_offset)
	    : Instruction {Tag::SetLocal}
	    , m_frame_offset {frame_offset} {}

	int m_frame_offset;
};

struct PushVariable : Instruction {
	PushVariable()
	    : Instruction {Tag::PushVariable} {}
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

struct NewBoolean : Instruction {
	NewBoolean(bool value)
	    : Instruction {Tag::NewBoolean}
	    , m_value {value} {}

	bool m_value;
};

struct NewNumber : Instruction {
	NewNumber(float value)
	    : Instruction {Tag::NewNumber}
	    , m_value {value} {}

	float m_value;
};

struct NewNull : Instruction {
	NewNull()
	    : Instruction {Tag::NewNull} {}
};

struct NewArray : Instruction {
	NewArray(int element_count)
	    : Instruction {Tag::NewArray}
	    , m_element_count {element_count} {}

	int m_element_count;
};

struct IndexAccess : Instruction {
	IndexAccess()
	    : Instruction {Tag::IndexAccess} {}
};

struct Jump : Instruction {
	Jump(int target_block)
	    : Instruction {Tag::Jump}
	    , m_target_block {target_block} {}

	int m_target_block;
};

struct JumpIfFalse : Instruction {
	JumpIfFalse(int target_block)
	    : Instruction {Tag::JumpIfFalse}
	    , m_target_block {target_block} {}

	int m_target_block;
};

struct Pop : Instruction {
	Pop()
	    : Instruction {Tag::Pop} {}
};

struct SaveReturn : Instruction {
	SaveReturn()
	    : Instruction {Tag::SaveReturn} {}
};

struct FetchReturn : Instruction {
	FetchReturn()
	    : Instruction {Tag::FetchReturn} {}
};

struct StartRegion : Instruction {
	StartRegion()
	    : Instruction {Tag::StartRegion} {}
};

struct EndRegion : Instruction {
	EndRegion()
	    : Instruction {Tag::EndRegion} {}
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
