#include "utils.hpp"

#include "../log/log.hpp"
#include "bytecode.hpp"
#include "eval.hpp"
#include "garbage_collector.hpp"
#include "interpreter.hpp"
#include "value.hpp"

namespace Interpreter {

void eval_call_function(Function* callee, int arg_count, Interpreter& e) {

	// TODO: error handling ?
	if (callee->m_def->m_args.size() != arg_count) {
		Log::internal_error() << "Function argument count mismatch";
	}

	e.m_stack.start_frame(0);

	for (int i = 0; i < arg_count; ++i) {
		auto value = e.m_stack.access(arg_count - 1 - i);
		e.m_stack.push_local(e.m_gc->new_variable_raw(value));
	}

	for (auto capture : callee->m_captures) {
		e.m_stack.push_local(capture);
	}

	if (!callee->m_def->tried_compilation) {
		callee->m_def->tried_compilation = true;

		Writer<Bytecode::Executable> bytecode = Bytecode::compile(callee->m_def->m_body);
		if (bytecode.ok()) {
			callee->m_def->bytecode =
			    new Bytecode::Executable {std::move(bytecode.m_result)};
		}
	}

	if (callee->m_def->bytecode) {
		Bytecode::execute(*callee->m_def->bytecode, e);
	} else {
		eval(callee->m_def->m_body, e);
	}

	auto result = e.m_stack.pop();
	e.m_stack.pop(); // callable
	for (int i = 0; i < arg_count; ++i) {
		e.m_stack.pop();
	}
	e.m_stack.push(result);

	e.m_stack.end_frame();
}

void eval_call_native_function(NativeFunction* callee, int arg_count, Interpreter& e) {
	auto args = e.m_stack.stack_range(arg_count, arg_count);
	auto result = callee(args, e);
	e.m_stack.pop(); // callable
	for (int i = 0; i < arg_count; ++i) {
		e.m_stack.pop();
	}
	e.m_stack.push(result);
}

void eval_call_callable(Value callee, int arg_count, Interpreter& e) {
	if (callee.type() == ValueTag::Function) {
		eval_call_function(callee.as<Function>(), arg_count, e);
	} else if (callee.type() == ValueTag::NativeFunction) {
		eval_call_native_function(callee.get_native_func(), arg_count, e);
	} else {
		Log::fatal("Attempted to call a non function at runtime");
	}
}




} // namespace Interpreter
