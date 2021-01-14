#pragma once

#include <vector>

#include "../utils/interned_string.hpp"

enum class Opcode {
	IntConst,
	FnConst,

	GlobalCreate,
	GlobalAccess,
	FrameAccess,

	PushFrame,
	PopFrame,
	PushRegion,
	PopRegion,
	SaveRetval,
	FetchRetval,
	Pop,

	SlideToFront,
	SlideToBack,


	Assign,

	Call,
	Ret,
};

struct Instruction {
	Opcode opcode;
	int int_value;
	InternedString string_value;
	std::vector<Instruction> fn_value;
};
