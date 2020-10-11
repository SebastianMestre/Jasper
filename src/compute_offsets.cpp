#include "compute_offsets.hpp"

#include "typed_ast.hpp"

#include <iostream>

#include <cassert>

namespace TypeChecker {

void compute_offsets(TypedAST::Declaration* ast, int frame_offset) {
	if (ast->m_value)
		compute_offsets(ast->m_value.get(), frame_offset);
}

void compute_offsets(TypedAST::Identifier* ast, int frame_offset) {
	TypedAST::Declaration* decl = ast->m_declaration;
	ast->m_frame_offset = decl->m_frame_offset;
}

//TODO
void compute_offsets(TypedAST::Block* ast, int frame_offset) {
	for (auto& child : ast->m_body) {
		if (child->type() == TypedASTTag::Declaration) {
			auto decl = static_cast<TypedAST::Declaration*>(child.get());
			decl->m_frame_offset = frame_offset++;
		}
		compute_offsets(child.get(), frame_offset);
	}
}

void compute_offsets(TypedAST::IfElseStatement* ast, int frame_offset) {
	compute_offsets(ast->m_condition.get(), frame_offset);
	compute_offsets(ast->m_body.get(), frame_offset);
	if (ast->m_else_body)
		compute_offsets(ast->m_else_body.get(), frame_offset);
}

void compute_offsets(TypedAST::CallExpression* ast, int frame_offset) {
	compute_offsets(ast->m_callee.get(), frame_offset);
	for (auto& arg : ast->m_args)
		compute_offsets(arg.get(), frame_offset);
}

void compute_offsets(TypedAST::FunctionLiteral* ast, int frame_offset) {
	// functions always start a new frame
	frame_offset = 0;

	for (auto& arg_decl : ast->m_args)
		arg_decl.m_frame_offset = frame_offset++;

	// scan body
	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body.get());
	compute_offsets(body, frame_offset);
}

void compute_offsets(TypedAST::ArrayLiteral* ast, int frame_offset) {
	for (auto& element : ast->m_elements)
		compute_offsets(element.get(), frame_offset);
}

void compute_offsets(TypedAST::ForStatement* ast, int frame_offset) {
	auto decl = static_cast<TypedAST::Declaration*>(ast->m_declaration.get());
	decl->m_frame_offset = frame_offset++;
	compute_offsets(ast->m_declaration.get(), frame_offset);
	compute_offsets(ast->m_condition.get(), frame_offset+1);
	compute_offsets(ast->m_action.get(), frame_offset+1);
	compute_offsets(ast->m_body.get(), frame_offset+1);
}

void compute_offsets(TypedAST::WhileStatement* ast, int frame_offset) {
	compute_offsets(ast->m_condition.get(), frame_offset);
	compute_offsets(ast->m_body.get(), frame_offset);
}

void compute_offsets(TypedAST::ReturnStatement* ast, int frame_offset) {
	compute_offsets(ast->m_value.get(), frame_offset);
}

void compute_offsets(TypedAST::IndexExpression* ast, int frame_offset) {
	compute_offsets(ast->m_callee.get(), frame_offset);
	compute_offsets(ast->m_index.get(), frame_offset);
}

void compute_offsets(TypedAST::TernaryExpression* ast, int frame_offset) {
	compute_offsets(ast->m_condition.get(), frame_offset);
	compute_offsets(ast->m_then_expr.get(), frame_offset);
	compute_offsets(ast->m_else_expr.get(), frame_offset);
}

void compute_offsets(TypedAST::RecordAccessExpression* ast, int frame_offset) {
	compute_offsets(ast->m_record.get(), frame_offset);
}

void compute_offsets(TypedAST::DeclarationList* ast, int frame_offset) {
	for (auto& decl : ast->m_declarations) {
		if (decl->m_value)
			compute_offsets(decl->m_value.get(), frame_offset);
	}
}

void compute_offsets(TypedAST::StructExpression* ast, int frame_offset) {
	for (auto& type : ast->m_types)
		compute_offsets(type.get(), frame_offset);
}

void compute_offsets(TypedAST::TypeTerm* ast, int frame_offset) {
	compute_offsets(ast->m_callee.get(), frame_offset);
	for (auto& arg : ast->m_args)
		compute_offsets(arg.get(), frame_offset);
}

void compute_offsets(TypedAST::TypedAST* ast, int frame_offset) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return compute_offsets(static_cast<TypedAST::type*>(ast), frame_offset);

#define DO_NOTHING(type)                                                       \
	case TypedASTTag::type:                                                    \
		return;

#define REJECT(type)                                                           \
	case TypedASTTag::type:                                                    \
		assert(0);

	switch (ast->type()) {
		DO_NOTHING(NumberLiteral);
		DO_NOTHING(IntegerLiteral);
		DO_NOTHING(StringLiteral);
		DO_NOTHING(BooleanLiteral);
		DO_NOTHING(NullLiteral);
		DISPATCH(ArrayLiteral);
		DISPATCH(FunctionLiteral);

		DISPATCH(Identifier);
		DISPATCH(IndexExpression);
		DISPATCH(CallExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(RecordAccessExpression);

		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}

#undef REJECT
#undef DO_NOTHING
#undef DISPATCH
	std::cerr << "INTERNAL ERROR: UNHANDLED CASE IN " << __PRETTY_FUNCTION__
	          << ": " << typed_ast_string[(int)ast->type()] << '\n';
	assert(0);
}

} // namespace TypeChecker
