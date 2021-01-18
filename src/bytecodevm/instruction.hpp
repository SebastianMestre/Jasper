#pragma once

#include <vector>

#include "../utils/interned_string.hpp"

#define OPCODES                                                                \
	X(IntConst)                                                                \
	X(StringConst)                                                             \
	X(FnConst)                                                                 \
                                                                               \
	X(GlobalCreate)                                                            \
	X(GlobalAccess)                                                            \
	X(FrameAccess)                                                             \
                                                                               \
	X(PushFrame)                                                               \
	X(PopFrame)                                                                \
	X(PushRegion)                                                              \
	X(PopRegion)                                                               \
	X(SaveRetval)                                                              \
	X(FetchRetval)                                                             \
	X(Pop)                                                                     \
                                                                               \
	X(Assign)                                                                  \
                                                                               \
	X(Call)                                                                    \
	X(Ret)                                                                     \
                                                                               \
	X(Jump)                                                                    \
	X(CondJump)

#define X(tag) tag,
enum class Opcode {
	OPCODES
};
#undef X

#define X(tag) #tag,
constexpr char const* opcode_string[] = {
	OPCODES
};
#undef X

struct Instruction {
	Opcode opcode;
	int int_value1;
	int int_value2;
	InternedString string_value;
	std::vector<Instruction> fn_value;
};
