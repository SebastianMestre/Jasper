#include "run_bytecode.hpp"

#include "../interpreter/interpreter.hpp"
#include "../interpreter/utils.hpp"
#include "../interpreter/value.hpp"
#include "../interpreter/value_tag.hpp"
#include "../log/log.hpp"
#include "bytecode.hpp"

#include <iostream>

int BytecodeRunner::run(std::vector<Instruction> const& instructions) {
	int const instruction_count = instructions.size();
	for (int i = 0; i < instruction_count;) {
		auto const& itn = instructions[i];
		int status = run_single(itn, i);
		if (status)
			return status;
	}
	return 0;
}

int BytecodeRunner::run_single(Instruction const& instruction, int& pc) {
	Log::info(std::string("==== Opcode ") + opcode_string[int(instruction.opcode)] + " ====");
	switch (instruction.opcode) {
	case Opcode::IntConst:
		e.push_integer(instruction.int_value1);
		break;
	case Opcode::FnConst: {
		// TODO
		int argument_count = instruction.int_value1;
		int capture_count = instruction.int_value2;
		Interpreter::CapturesType captures(capture_count, nullptr);
		for (int i = capture_count; i-- > 0;)
			captures[i] = e.m_stack.pop_unsafe();
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

		int argument_count = instruction.int_value1;
		int frame_start = e.m_stack.m_stack_ptr - argument_count;

		Log::info("argument count = " + std::to_string(argument_count));

		auto callee = Interpreter::value_of(e.m_stack.access(argument_count));
		if (callee->type() == ValueTag::NativeFunction) {
			auto typed_callee = static_cast<Interpreter::NativeFunction*>(callee);

			e.m_stack.start_stack_frame(frame_start);
			Span<Interpreter::Value*> args = {
			    &e.m_stack.frame_at(0), argument_count};
			e.m_stack.frame_at(-1) = typed_callee->m_fptr(args, e);
			e.m_stack.end_stack_frame();
		} else if (callee->type() == ValueTag::BytecodeFunction) {
			auto typed_callee = static_cast<Interpreter::BytecodeFunction*>(callee);

			for (int i = 0; i < argument_count; ++i)
				e.m_stack.access(i) = e.new_reference(value_of(e.m_stack.access(i))).get();

			for (auto capture : typed_callee->m_captures)
				e.m_stack.push(capture);

			e.m_stack.start_stack_frame(frame_start);
			// TODO eliminate recursion
			run(*typed_callee->m_def);
			e.m_stack.frame_at(-1) = e.m_stack.pop_unsafe();
			e.m_stack.end_stack_frame();
		} else {
			Log::FatalStream() << "invalid callable -- type is "
			                   << value_string[int(callee->type())];
		}

	} break;
	}
	switch(instruction.opcode){
		case Opcode::Jump:
			pc += instruction.int_value1;
			break;
	    case Opcode::CondJump: {
		    bool success =
		        Interpreter::value_as<Interpreter::Boolean>(e.m_stack.access(0))->m_value;
		    e.m_stack.pop_unsafe();
		    if (success)
			    pc += instruction.int_value1;
	    } break;
	    default: pc += 1;
	}
	return 0;
}
