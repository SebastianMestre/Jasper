#include "bytecode.hpp"

#include <cstring>

template <typename T>
Writer<T> make_writer(T x) {
	return {{}, std::move(x)};
}

namespace Bytecode {

ErrorReport visit(BasicBlock&, AST::Expr*);

ErrorReport success() { return {}; }
ErrorReport failure() { return {"Failed to generate bytecode"}; }

Writer<Executable> compile(AST::Expr* expr) {
	Executable result;
	BasicBlock main_block;
	ErrorReport status = visit(main_block, expr);
	if (status.ok()) {
	} else {
		return status;
	}

	result.blocks.push_back(std::move(main_block));
	return make_writer(std::move(result));
}

template<typename InstructionType>
void emit_instruction(BasicBlock& b, InstructionType instruction) {
	constexpr auto byte_count = sizeof(instruction);
	char buffer[byte_count];
	memcpy(buffer, &instruction, byte_count);
	for (int i = 0; i < byte_count; ++i)
		b.bytecode.push_back(buffer[i]);
}

ErrorReport compile_identifier(BasicBlock& b, AST::Identifier* expr) {
	if (expr->m_origin != AST::Identifier::Origin::Global)
		return failure();

	emit_instruction(b, GetGlobal {expr->m_text});
	return success();
}

ErrorReport compile_integer_literal(BasicBlock& b, AST::IntegerLiteral* expr) {
	emit_instruction(b, NewInteger {expr->m_value});
	return success();
}

ErrorReport visit(BasicBlock& b, AST::Expr* expr) {
	switch (expr->type()) {
	case AST::ExprTag::Identifier:
		return compile_identifier(b, static_cast<AST::Identifier*>(expr));
	case AST::ExprTag::IntegerLiteral:
		return compile_integer_literal(b, static_cast<AST::IntegerLiteral*>(expr));
	}
	return failure();
}

int decode(char const* stream, Interpreter::Interpreter& e) {
	Instruction const* punned = reinterpret_cast<Instruction const*>(stream);
	switch (punned->tag()) {
	case Instruction::Tag::NewInteger: {
		auto op = static_cast<NewInteger const*>(punned);
		e.push_integer(op->m_value);
		return sizeof(*op);
	}
	case Instruction::Tag::GetGlobal: {
		auto op = static_cast<GetGlobal const*>(punned);
		e.m_stack.push(Interpreter::Value{e.global_access(op->m_name)});
		return sizeof(*op);
	}
	}
	return 1024;
}

void execute(Executable const& exe, Interpreter::Interpreter& e) {
	int cursor = 0;
	while (cursor < exe.blocks[0].bytecode.size()) {
		cursor += decode(&exe.blocks[0].bytecode[cursor], e);
	}
}

} // namespace Bytecode
