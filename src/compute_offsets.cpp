#include "compute_offsets.hpp"

#include "typed_ast.hpp"

#include <iostream>

#include <cassert>

namespace TypeChecker {

void compute_offsets(TypedAST::Declaration* ast, int frame_offset) {
	if (ast->m_value)
		compute_offsets(ast->m_value, frame_offset);
}

void compute_offsets(TypedAST::Identifier* ast, int frame_offset) {
	TypedAST::Declaration* decl = ast->m_declaration;
	if (ast->m_origin == TypedAST::Identifier::Origin::Local) {
		ast->m_frame_offset = decl->m_frame_offset;
	} else if (ast->m_origin == TypedAST::Identifier::Origin::Capture) {
		auto& capture_data = ast->m_surrounding_function->m_captures[ast->text()];
		ast->m_frame_offset = capture_data.inner_frame_offset;
	} else {
		return;
	}
}

//TODO
void compute_offsets(TypedAST::Block* ast, int frame_offset) {
	for (auto& child : ast->m_body) {
		if (child->type() == TypedASTTag::Declaration) {
			auto decl = static_cast<TypedAST::Declaration*>(child);
			decl->m_frame_offset = frame_offset++;
		}
		compute_offsets(child, frame_offset);
	}
}

void compute_offsets(TypedAST::IfElseStatement* ast, int frame_offset) {
	compute_offsets(ast->m_condition, frame_offset);
	compute_offsets(ast->m_body, frame_offset);
	if (ast->m_else_body)
		compute_offsets(ast->m_else_body, frame_offset);
}

void compute_offsets(TypedAST::CallExpression* ast, int frame_offset) {
	compute_offsets(ast->m_callee, frame_offset);
	for (auto& arg : ast->m_args)
		compute_offsets(arg, frame_offset);
}

void compute_offsets(TypedAST::FunctionLiteral* ast, int frame_offset) {
	// functions start a new frame
	frame_offset = 0;

	// first thing in a frame: arguments
	for (auto& arg_decl : ast->m_args)
		arg_decl.m_frame_offset = frame_offset++;

	// second thing: captures
	for (auto& kv : ast->m_captures) {
		kv.second.inner_frame_offset = frame_offset++;
		auto decl = kv.second.outer_declaration;
		if (decl->m_surrounding_function == ast->m_surrounding_function) {
			// capture of a local variable
			// just use the frame offset of the declaration
			kv.second.outer_frame_offset = decl->m_frame_offset;
		} else {
			// capture of a capture
			// look at the captures of the surrounding function
			kv.second.outer_frame_offset =
			    ast->m_surrounding_function->m_captures[kv.first].inner_frame_offset;
		}
	}

	// TODO? store the frame size

	// scan body
	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body);
	compute_offsets(body, frame_offset);
}

void compute_offsets(TypedAST::ArrayLiteral* ast, int frame_offset) {
	for (auto& element : ast->m_elements)
		compute_offsets(element, frame_offset);
}

void compute_offsets(TypedAST::ForStatement* ast, int frame_offset) {
	auto& decl = ast->m_declaration;
	decl.m_frame_offset = frame_offset++;
	compute_offsets(&ast->m_declaration, frame_offset);
	compute_offsets(ast->m_condition, frame_offset+1);
	compute_offsets(ast->m_action, frame_offset+1);
	compute_offsets(ast->m_body, frame_offset+1);
}

void compute_offsets(TypedAST::WhileStatement* ast, int frame_offset) {
	compute_offsets(ast->m_condition, frame_offset);
	compute_offsets(ast->m_body, frame_offset);
}

void compute_offsets(TypedAST::ReturnStatement* ast, int frame_offset) {
	compute_offsets(ast->m_value, frame_offset);
}

void compute_offsets(TypedAST::IndexExpression* ast, int frame_offset) {
	compute_offsets(ast->m_callee, frame_offset);
	compute_offsets(ast->m_index, frame_offset);
}

void compute_offsets(TypedAST::TernaryExpression* ast, int frame_offset) {
	compute_offsets(ast->m_condition, frame_offset);
	compute_offsets(ast->m_then_expr, frame_offset);
	compute_offsets(ast->m_else_expr, frame_offset);
}

void compute_offsets(TypedAST::RecordAccessExpression* ast, int frame_offset) {
	compute_offsets(ast->m_record, frame_offset);
}

void compute_offsets(TypedAST::DeclarationList* ast, int frame_offset) {
	for (auto& decl : ast->m_declarations) {
		if (decl.m_value)
			compute_offsets(decl.m_value, frame_offset);
	}
}

void compute_offsets(TypedAST::StructExpression* ast, int frame_offset) {
	for (auto& type : ast->m_types)
		compute_offsets(type, frame_offset);
}

void compute_offsets(TypedAST::TypeTerm* ast, int frame_offset) {
	compute_offsets(ast->m_callee, frame_offset);
	for (auto& arg : ast->m_args)
		compute_offsets(arg, frame_offset);
}

void compute_offsets(TypedAST::TypedAST* ast, int frame_offset) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return compute_offsets(static_cast<TypedAST::type*>(ast), frame_offset);

#define DO_NOTHING(type)                                                       \
	case TypedASTTag::type:                                                    \
		return;

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

#undef DO_NOTHING
#undef DISPATCH
	std::cerr << "INTERNAL ERROR: UNHANDLED CASE IN " << __PRETTY_FUNCTION__
	          << ": " << typed_ast_string[(int)ast->type()] << '\n';
	assert(0);
}

} // namespace TypeChecker
