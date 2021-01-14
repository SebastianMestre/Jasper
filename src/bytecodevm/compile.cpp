#include "compile.hpp"

#include <iostream>

#include "bytecode.hpp"
#include "../log/log.hpp"
#include "../typed_ast.hpp"

static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::TypedAST* ast);

static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::IntegerLiteral* ast) {
	out.push_back(Instruction {Opcode::IntConst, ast->value()});
}

static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::FunctionLiteral* ast) {
	auto body_bytecode = emit_bytecode(ast->m_body);
	out.push_back(Instruction {Opcode::FnConst, ast->m_args.size(), "", std::move(body_bytecode)});
}

static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::Identifier* ast) {
	if (ast->m_origin == TypedAST::Identifier::Origin::Local ||
	    ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		if (ast->m_frame_offset == INT_MIN) {
			Log::FatalStream() << "MISSING LAYOUT FOR AN IDENTIFIER" << ast->text().str();
		}

		out.push_back(Instruction {Opcode::FrameAccess, ast->m_frame_offset});
	} else {
		out.push_back(Instruction {Opcode::GlobalAccess, 0, ast->text()});
	}
}

static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::CallExpression* ast) {
	emit_bytecode_impl(out, ast->m_callee);
	for (auto arg : ast->m_args)
		emit_bytecode_impl(out, arg);
	out.push_back(Instruction {Opcode::Call, ast->m_args.size()});
}

static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::DeclarationList* ast) {
	for (auto decl : ast->m_declarations) {
		out.push_back(
		    Instruction {Opcode::GlobalCreate, 0, decl.identifier_text()});
		emit_bytecode_impl(out, decl.m_value);
		out.push_back(Instruction {Opcode::Assign});
	}
}


static void emit_bytecode_impl(std::vector<Instruction>& out, TypedAST::TypedAST* ast) {
#define DISPATCH(tag)                                                          \
	case TypedASTTag::tag:                                                     \
		return emit_bytecode_impl(out, static_cast<TypedAST::tag*>(ast));

	switch (ast->type()) {
		DISPATCH(IntegerLiteral)
		DISPATCH(FunctionLiteral)

		DISPATCH(Identifier)
		DISPATCH(CallExpression)

		DISPATCH(DeclarationList)
	}

#undef DISPATCH
}


std::vector<Instruction> emit_bytecode(TypedAST::TypedAST* ast) {
	std::vector<Instruction> result;
	emit_bytecode_impl(result, ast);
	std::cerr << "Code generation produced " << result.size() << " bytecode instructions\n";
	return result;
}
