#include "utils.hpp"

#include "../log/log.hpp"
#include "bytecode.hpp"
#include "eval.hpp"
#include "interpreter.hpp"
#include "value.hpp"

namespace Interpreter {

Value value_of(Value h) {
	if (!is_heap_type(h.type()))
		return h;

	if (h.type() != ValueTag::Variable)
		return h;

	// try unboxing recursively?
	auto ref = h.get_cast<Variable>();
	return ref->m_value;
}


void eval_call_function(Function* callee, int arg_count, Interpreter& e) {

	for (int i = 0; i < arg_count; ++i) {
		auto ref = e.new_variable(Value {nullptr});
		ref->m_value = e.m_stack.access(i);
		e.m_stack.access(i) = ref.as_value();
	}

	// TODO: error handling ?
	assert(callee->m_def->m_args.size() == arg_count);

	if (!callee->m_def->tried_compilation) {
		callee->m_def->tried_compilation = true;

		Writer<Bytecode::Executable> bytecode = Bytecode::compile(callee->m_def->m_body);
		if (bytecode.ok()) {
			callee->m_def->bytecode =
			    new Bytecode::Executable {std::move(bytecode.m_result)};
		}
	}

	for (auto capture : callee->m_captures)
		e.m_stack.push(Value{capture});

	if (callee->m_def->bytecode) {
		Bytecode::execute(*callee->m_def->bytecode, e);
	} else {
		eval(callee->m_def->m_body, e);
	}

}

void eval_call_native_function(NativeFunction* callee, int arg_count, Interpreter& e) {
	auto args = e.m_stack.frame_range(0, arg_count);
	e.m_stack.push(callee(args, e));
}

void eval_call_callable(Value callee, int arg_count, Interpreter& e) {
	if (callee.type() == ValueTag::Function) {
		eval_call_function(callee.get_cast<Function>(), arg_count, e);
	} else if (callee.type() == ValueTag::NativeFunction) {
		eval_call_native_function(callee.get_native_func(), arg_count, e);
	} else {
		Log::fatal("Attempted to call a non function at runtime");
	}
}




} // namespace Interpreter
