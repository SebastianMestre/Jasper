#include "run_bytecode.hpp"

#include "../interpreter/interpreter.hpp"
#include "../interpreter/utils.hpp"
#include "../interpreter/value.hpp"
#include "../interpreter/value_tag.hpp"
#include "../log/log.hpp"
#include "bytecode.hpp"

#include <iostream>

int BytecodeRunner::run(std::vector<Instruction> const& instructions) {
	for (Instruction const& itn : instructions) {
		int status = run_single(itn);
		if (status)
			return status;
	}
	return 0;
}

int BytecodeRunner::run_single(Instruction const& instruction) {
	Log::info(std::string("==== Opcode ") + opcode_string[int(instruction.opcode)] + " ====");
	switch (instruction.opcode) {
	case Opcode::IntConst:
		e.push_integer(instruction.int_value1);
		break;
	case Opcode::FnConst: {
		// TODO
		int argument_count = instruction.int_value1;
		int capture_count = instruction.int_value2;
		Interpreter::CapturesType captures;
		for (int i = 0; i < capture_count; ++i)
			captures.push_back(e.m_stack.pop_unsafe());
		auto fn = e.new_bytecode_function(&instruction.fn_value, std::move(captures));
		e.m_stack.push(fn.get());
	} break;
	case Opcode::GlobalAccess:
		Log::info("identifier is '" + instruction.string_value.str() + "'");
		e.m_stack.push(e.global_access(instruction.string_value));
		break;
	case Opcode::GlobalCreate: {
		auto ref = e.new_reference(e.null());
		e.global_declare_direct(instruction.string_value, ref.get());
		e.m_stack.push(ref.get());
	} break;
	case Opcode::FrameAccess:
		e.m_stack.push(e.m_stack.frame_at(instruction.int_value1));
		break;
	case Opcode::Assign:
		e.assign(e.m_stack.peek(1).get(), e.m_stack.peek(0).get());
		e.m_stack.pop_unsafe();
		e.m_stack.pop_unsafe();
		break;
	case Opcode::PushRegion:
		e.m_stack.start_stack_region();
		break;
	case Opcode::PopRegion:
		e.m_stack.end_stack_region();
		break;
	case Opcode::Call: {
		std::cerr << "Call (with " << instruction.int_value << " arguments)\n";
		int arg_count = instruction.int_value;
		e.m_stack.start_stack_frame(e.m_stack.m_stack_ptr - arg_count);
		auto callee = Interpreter::value_as<Interpreter::NativeFunction>(e.m_stack.access(arg_count));
		Span<Interpreter::Value*> args = {&e.m_stack.frame_at(0), arg_count};
		e.m_stack.access(arg_count) = callee->m_fptr(args, e);
		for(int i = arg_count; i-- > 0;)
			e.m_stack.pop_unsafe();
	} break;
	}
	return 0;
}
