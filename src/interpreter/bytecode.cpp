#include "bytecode.hpp"

#include "utils.hpp"
#include "value.hpp"

#include <cstring>

template <typename T>
Writer<T> make_writer(T x) {
	return {{}, std::move(x)};
}

namespace Bytecode {

static ErrorReport success() { return {}; }
static ErrorReport failure() { return {"Failed to generate bytecode"}; }

struct BytecodeBuilder {
	int current_basic_block {-1};
	std::vector<BasicBlock> blocks;


	ErrorReport compile_identifier(AST::Identifier* expr) {
		if (expr->m_origin == AST::Identifier::Origin::Local ||
			expr->m_origin == AST::Identifier::Origin::Capture) {
			emit_instruction(GetLocal {expr->m_frame_offset});
			return success();
		} else {
			emit_instruction(GetGlobal {expr->m_text});
			return success();
		}
	}

	ErrorReport compile_call_expression(AST::CallExpression* expr) {
		auto status1 = visit(expr->m_callee);
		if (!status1.ok()) return status1;

		for (auto arg : expr->m_args) {
			auto status2 = visit(arg);
			if (!status2.ok()) return status2;
		}

		emit_instruction(Call {expr->m_args.size()});
		return success();
	}

	ErrorReport compile_integer_literal(AST::IntegerLiteral* expr) {
		emit_instruction(NewInteger {expr->m_value});
		return success();
	}

	ErrorReport compile_boolean_literal(AST::BooleanLiteral* expr) {
		emit_instruction(NewBoolean {expr->m_value});
		return success();
	}

	ErrorReport compile_number_literal(AST::NumberLiteral* expr) {
		emit_instruction(NewNumber {expr->value()});
		return success();
	}

	ErrorReport compile_null_literal(AST::NullLiteral* expr) {
		emit_instruction(NewNull {});
		return success();
	}

	ErrorReport compile_array_literal(AST::ArrayLiteral* expr) {
		for (auto element : expr->m_elements) {
			auto status = visit(element);
			if (!status.ok()) return status;
		}
		emit_instruction(NewArray {expr->m_elements.size()});
		return success();
	}

	ErrorReport compile_index_expression(AST::IndexExpression* expr) {
		auto status1 = visit(expr->m_callee);
		if (!status1.ok()) return status1;

		auto status2 = visit(expr->m_index);
		if (!status2.ok()) return status2;

		emit_instruction(IndexAccess {});
		return success();
	}

	ErrorReport compile_ternary_expression(AST::TernaryExpression* expr) {

		int then_block = new_block();
		int else_block = new_block();
		int after_block = new_block();

		auto status1 = visit(expr->m_condition);
		if (!status1.ok()) return status1;
		emit_instruction(JumpIfFalse {else_block});
		emit_instruction(Jump {then_block});

		set_current_block(then_block);
		auto status2 = visit(expr->m_then_expr);
		if (!status2.ok()) return status2;
		emit_instruction(Jump {after_block});

		set_current_block(else_block);
		auto status3 = visit(expr->m_else_expr);
		if (!status3.ok()) return status3;
		emit_instruction(Jump {after_block});

		set_current_block(after_block);

		return success();
	}

	ErrorReport visit(AST::Expr* expr) {
		switch (expr->type()) {
		case AST::ExprTag::Identifier:
			return compile_identifier(static_cast<AST::Identifier*>(expr));
		case AST::ExprTag::IntegerLiteral:
			return compile_integer_literal(static_cast<AST::IntegerLiteral*>(expr));
		case AST::ExprTag::BooleanLiteral:
			return compile_boolean_literal(static_cast<AST::BooleanLiteral*>(expr));
		case AST::ExprTag::NumberLiteral:
			return compile_number_literal(static_cast<AST::NumberLiteral*>(expr));
		case AST::ExprTag::NullLiteral:
			return compile_null_literal(static_cast<AST::NullLiteral*>(expr));
		case AST::ExprTag::ArrayLiteral:
			return compile_array_literal(static_cast<AST::ArrayLiteral*>(expr));
		case AST::ExprTag::IndexExpression:
			return compile_index_expression(static_cast<AST::IndexExpression*>(expr));
		case AST::ExprTag::CallExpression:
			return compile_call_expression(static_cast<AST::CallExpression*>(expr));
		case AST::ExprTag::TernaryExpression:
			return compile_ternary_expression(static_cast<AST::TernaryExpression*>(expr));
		}
		return failure();
	}

	template<typename InstructionType>
	void emit_instruction(InstructionType instruction) {
		auto& block = blocks[current_basic_block];
		auto& bytecode = block.bytecode;
		auto byte_count = sizeof(instruction);
		char buffer[byte_count];
		memcpy(buffer, &instruction, byte_count);
		for (int i = 0; i < byte_count; ++i)
			bytecode.push_back(buffer[i]);
	}

	int new_block() {
		blocks.push_back(BasicBlock {});
		return blocks.size() - 1;
	}

	int set_current_block(int block) {
		int old_block = current_basic_block;
		current_basic_block = block;
		return old_block;
	}
};


Writer<Executable> compile(AST::Expr* expr) {
	BytecodeBuilder builder;
	Executable result;
	int main_block = builder.new_block();
	builder.set_current_block(main_block);
	ErrorReport status = builder.visit(expr);
	if (!status.ok()) {
		return status;
	}

	result.blocks = std::move(builder.blocks);
	return make_writer(std::move(result));
}



static int decode(char const* stream, Interpreter::Interpreter& e) {
	Instruction const* punned = reinterpret_cast<Instruction const*>(stream);
	switch (punned->tag()) {
	case Instruction::Tag::GetGlobal: {
		auto op = static_cast<GetGlobal const*>(punned);
		e.m_stack.push(e.global_access(op->m_name)->m_value);
		return sizeof(*op);
	}
	case Instruction::Tag::GetLocal: {
		auto op = static_cast<GetLocal const*>(punned);
		e.m_stack.push(e.m_stack.frame_at(op->m_frame_offset).as<Interpreter::Variable>()->m_value);
		return sizeof(*op);
	}
	case Instruction::Tag::NewInteger: {
		auto op = static_cast<NewInteger const*>(punned);
		e.push_integer(op->m_value);
		return sizeof(*op);
	}
	case Instruction::Tag::NewBoolean: {
		auto op = static_cast<NewBoolean const*>(punned);
		e.push_boolean(op->m_value);
		return sizeof(*op);
	}
	case Instruction::Tag::NewNumber: {
		auto op = static_cast<NewNumber const*>(punned);
		e.push_float(op->m_value);
		return sizeof(*op);
	}
	case Instruction::Tag::NewNull: {
		auto op = static_cast<NewNull const*>(punned);
		e.m_stack.push(e.null());
		return sizeof(*op);
	}
	case Instruction::Tag::NewArray: {
		auto op = static_cast<NewArray const*>(punned);
		e.push_list({});
		int element_count = op->m_element_count;
		auto result = e.m_stack.access(0).as<Interpreter::Array>();
		result->m_value.reserve(element_count);
		for (int i = 0; i < element_count; ++i) {
			result->append(e.m_stack.access(element_count - i));
		}
		// now we remove the elements from beneath the array, leaving only the array on the stack
		e.m_stack.access(element_count) = e.m_stack.pop();
		for (int i = 0; i < element_count - 1; ++i) {
			e.m_stack.pop();
		}
		return sizeof(*op);
	}
	case Instruction::Tag::IndexAccess: {
		auto op = static_cast<IndexAccess const*>(punned);
		auto index = e.m_stack.pop().get_integer();
		auto callee_ptr = e.m_stack.pop();
		auto* callee = callee_ptr.as<Interpreter::Array>();
		e.m_stack.push(callee->at(index));
		return sizeof(*op);
	}
	case Instruction::Tag::Call: {
		auto op = static_cast<Call const*>(punned);
		int argument_count = op->m_argument_count;

		auto callee = e.m_stack.access(argument_count);

		e.m_stack.start_frame(argument_count);

		eval_call_callable(callee, argument_count, e);

		e.m_stack.frame_at(-1) = e.m_stack.pop();
		e.m_stack.end_frame();

		return sizeof(*op);
	}
	case Instruction::Tag::Jump: {
		auto op = static_cast<Jump const*>(punned);
		return -op->m_target_block;
	}
	case Instruction::Tag::JumpIfFalse: {
		auto op = static_cast<JumpIfFalse const*>(punned);
		if (e.m_stack.pop().get_boolean() == false) {
			return -op->m_target_block;
		} else {
			return sizeof(*op);
		}
	}
	}
	return 1024;
}

void execute(Executable const& exe, Interpreter::Interpreter& e) {
	int current_block = 0;
	int cursor = 0;
	while (cursor < exe.blocks[current_block].bytecode.size()) {
		int offset = decode(&exe.blocks[current_block].bytecode[cursor], e);
		if (offset < 0) { // this indicates we need to jump to a different block
			int target_block = -offset;
			current_block = target_block;
			cursor = 0;
		} else {
			cursor += offset;
		}
	}
}

} // namespace Bytecode
