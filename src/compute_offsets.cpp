#include "compute_offsets.hpp"

#include "./log/log.hpp"
#include "ast.hpp"

#include <cassert>

namespace TypeChecker {

static void process_stmt(AST::Stmt* ast, int frame_offset);

void compute_offsets(AST::Identifier* ast, int frame_offset) {
	AST::Declaration* decl = ast->m_declaration;
	if (ast->m_origin == AST::Identifier::Origin::Local) {
		ast->m_frame_offset = decl->m_frame_offset;
	} else if (ast->m_origin == AST::Identifier::Origin::Capture) {
		auto& capture_data = ast->m_surrounding_function->m_captures[ast->text()];
		ast->m_frame_offset = capture_data.inner_frame_offset;
	} else {
		return;
	}
}


void compute_offsets(AST::CallExpression* ast, int frame_offset) {
	compute_offsets(ast->m_callee, frame_offset++);
	for (auto& arg : ast->m_args)
		compute_offsets(arg, frame_offset++);
}

void compute_offsets(AST::FunctionLiteral* ast, int frame_offset) {
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
	compute_offsets(ast->m_body, frame_offset);
}

void compute_offsets(AST::ArrayLiteral* ast, int frame_offset) {
	for (auto& element : ast->m_elements)
		compute_offsets(element, frame_offset);
}


static void process_stmt(AST::Declaration* ast, int frame_offset) {
	if (ast->m_value)
		compute_offsets(ast->m_value, frame_offset);
}

static void process_stmt(AST::Block* ast, int frame_offset) {
	for (auto& child : ast->m_body) {
		if (child->tag() == ASTStmtTag::Declaration) {
			auto decl = static_cast<AST::Declaration*>(child);
			decl->m_frame_offset = frame_offset++;
		}
		process_stmt(child, frame_offset);
	}
}

static void process_stmt(AST::IfElseStatement* ast, int frame_offset) {
	compute_offsets(ast->m_condition, frame_offset);
	process_stmt(ast->m_body, frame_offset);
	if (ast->m_else_body)
		process_stmt(ast->m_else_body, frame_offset);
}

static void process_stmt(AST::WhileStatement* ast, int frame_offset) {
	compute_offsets(ast->m_condition, frame_offset);
	process_stmt(ast->m_body, frame_offset);
}

static void process_stmt(AST::ReturnStatement* ast, int frame_offset) {
	compute_offsets(ast->m_value, frame_offset);
}

static void process_stmt(AST::ExpressionStatement* ast, int frame_offset) {
	compute_offsets(ast->m_expression, frame_offset);
}

static void process_stmt(AST::Stmt* ast, int frame_offset) {
#define DISPATCH(type)                                                         \
	case ASTStmtTag::type:                                                     \
		return process_stmt(static_cast<AST::type*>(ast), frame_offset);

	switch (ast->tag()) {
		DISPATCH(Block);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);
		DISPATCH(ExpressionStatement);
		DISPATCH(Declaration);
	}

#undef DISPATCH
	Log::fatal() << "(internal) Unhandled case in compute_offsets/process_stmt ("
	             << ast_string[(int)ast->type()] << ")";
}


void compute_offsets(AST::IndexExpression* ast, int frame_offset) {
	compute_offsets(ast->m_callee, frame_offset);
	compute_offsets(ast->m_index, frame_offset);
}

void compute_offsets(AST::TernaryExpression* ast, int frame_offset) {
	compute_offsets(ast->m_condition, frame_offset);
	compute_offsets(ast->m_then_expr, frame_offset);
	compute_offsets(ast->m_else_expr, frame_offset);
}

void compute_offsets(AST::AccessExpression* ast, int frame_offset) {
	compute_offsets(ast->m_target, frame_offset);
}

void compute_offsets(AST::MatchExpression* ast, int frame_offset) {
	compute_offsets(&ast->m_target, frame_offset);

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		// Declarations never have values: no need to compute_offsets() on them
		case_data.m_declaration.m_frame_offset = frame_offset;

		// We put the identifier that's bound to the matched value in the first
		// position, so when we recurse on the children, we add 1 to the offset

		compute_offsets(case_data.m_expression, frame_offset+1);
	}
}

void compute_offsets(AST::ConstructorExpression* ast, int frame_offset) {
	compute_offsets(ast->m_constructor, frame_offset);

	for (auto& arg : ast->m_args)
		compute_offsets(arg, frame_offset);
}

void compute_offsets(AST::SequenceExpression* ast, int frame_offset) {
	process_stmt(ast->m_body, frame_offset);
}

void compute_offsets(AST::StructExpression* ast, int frame_offset) {
	for (auto& type : ast->m_types)
		compute_offsets(type, frame_offset);
}

void compute_offsets(AST::UnionExpression* ast, int frame_offset) {
	for (auto& type : ast->m_types)
		compute_offsets(type, frame_offset);
}

void compute_offsets(AST::TypeTerm* ast, int frame_offset) {
	compute_offsets(ast->m_callee, frame_offset);
	for (auto& arg : ast->m_args)
		compute_offsets(arg, frame_offset);
}

void compute_offsets(AST::AST* ast, int frame_offset) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return compute_offsets(static_cast<AST::type*>(ast), frame_offset);

#define DO_NOTHING(type)                                                       \
	case ASTTag::type:                                                    \
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
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		DISPATCH(StructExpression);
		DISPATCH(UnionExpression);
		DISPATCH(TypeTerm);

		DO_NOTHING(BuiltinTypeFunction);
		DO_NOTHING(Constructor);
	}

#undef DO_NOTHING
#undef DISPATCH
	Log::fatal() << "(internal) Unhandled case in compute_offsets ("
	             << ast_string[(int)ast->type()] << ")";
}

void compute_offsets_program(AST::Program* ast, int frame_offset) {
	for (auto& decl : ast->m_declarations) {
		if (decl.m_value)
			compute_offsets(decl.m_value, frame_offset);
	}
}

} // namespace TypeChecker
