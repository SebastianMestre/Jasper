#include "metacheck.hpp"

#include "typechecker.hpp"
#include "typed_ast.hpp"

#include <iostream>
#include <cassert>

namespace TypeChecker {

// literals

void metacheck_literal(TypedAST::TypedAST* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
}

void metacheck(TypedAST::ArrayLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	for (auto& element : ast->m_elements) {
		metacheck(element.get(), tc);
		tc.m_meta_core.unify(element->m_meta_type, tc.meta_value());
	}
}

// expressions

void metacheck(TypedAST::Identifier* ast, TypeChecker& tc) {
	assert(ast->m_declaration);
	ast->m_meta_type = ast->m_declaration->m_meta_type;
}

void metacheck(TypedAST::FunctionLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	int arg_count = ast->m_args.size();
	for (int i = 0; i < arg_count; ++i) {
		// TODO: metacheck type hints
		auto& arg_decl = ast->m_args[i];
		arg_decl.m_meta_type = tc.meta_value();
	}

	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body.get());
	for (auto& child : body->m_body)
		metacheck(child.get(), tc);
}

void metacheck(TypedAST::CallExpression* ast, TypeChecker& tc) {
	// TODO? maybe we would like to support compile time functions that return types eventually?
	ast->m_meta_type = tc.meta_value();

	metacheck(ast->m_callee.get(), tc);
	tc.m_meta_core.unify(ast->m_callee->m_meta_type, tc.meta_value());

	for (auto& arg : ast->m_args) {
		metacheck(arg.get(), tc);
		tc.m_meta_core.unify(arg->m_meta_type, tc.meta_value());
	}
}

void metacheck(TypedAST::IndexExpression* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	metacheck(ast->m_callee.get(), tc);
	tc.m_meta_core.unify(ast->m_callee->m_meta_type, tc.meta_value());

	metacheck(ast->m_index.get(), tc);
	tc.m_meta_core.unify(ast->m_index->m_meta_type, tc.meta_value());
}

void metacheck(TypedAST::TernaryExpression* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	metacheck(ast->m_condition.get(), tc);
	tc.m_meta_core.unify(ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_then_expr.get(), tc);
	tc.m_meta_core.unify(ast->m_then_expr->m_meta_type, tc.meta_value());

	metacheck(ast->m_else_expr.get(), tc);
	tc.m_meta_core.unify(ast->m_else_expr->m_meta_type, tc.meta_value());
}

void metacheck(TypedAST::RecordAccessExpression* ast, TypeChecker& tc) {
	// TODO: we would like to support static records with
	// typefunc members in the future
	ast->m_meta_type = tc.meta_value();

	metacheck(ast->m_record.get(), tc);
	tc.m_meta_core.unify(ast->m_record->m_meta_type, tc.meta_value());
}

// statements

void metacheck(TypedAST::Block* ast, TypeChecker& tc) {
	for (auto& child : ast->m_body)
		metacheck(child.get(), tc);
}

void metacheck(TypedAST::IfElseStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_condition.get(), tc);
	tc.m_meta_core.unify(ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_body.get(), tc);

	if (ast->m_else_body)
		metacheck(ast->m_else_body.get(), tc);
}

void metacheck(TypedAST::ForStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_declaration.get(), tc);
	metacheck(ast->m_condition.get(), tc);
	tc.m_meta_core.unify(
	    ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_action.get(), tc);
	metacheck(ast->m_body.get(), tc);
}

void metacheck(TypedAST::WhileStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_condition.get(), tc);
	tc.m_meta_core.unify(
	    ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_body.get(), tc);
}

void metacheck(TypedAST::ReturnStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_value.get(), tc);
}

// declarations

void metacheck(TypedAST::Declaration* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.new_meta_var();
	metacheck(ast->m_value.get(), tc);
	tc.m_meta_core.unify(ast->m_meta_type, ast->m_value->m_meta_type);
}

void metacheck(TypedAST::DeclarationList* ast, TypeChecker& tc) {
	for (auto& decl : ast->m_declarations)
		decl->m_meta_type = tc.new_meta_var();

	for (auto& decl : ast->m_declarations)
		metacheck(decl->m_value.get(), tc);

	for (auto& decl : ast->m_declarations)
		tc.m_meta_core.unify(decl->m_meta_type, decl->m_value->m_meta_type);

	for (auto& decl : ast->m_declarations)
		if (tc.m_meta_core.find(decl->m_meta_type) == tc.meta_typefunc())
			for (auto* other : decl->m_references)
				if (tc.m_meta_core.find(other->m_meta_type) == tc.meta_value())
					assert(0 && "value referenced in a type definition");
}

void metacheck(TypedAST::StructExpression* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_typefunc();

	for (auto& type : ast->m_types) {
		metacheck(type.get(), tc);
		tc.m_meta_core.unify(type->m_meta_type, tc.meta_monotype());
	}
}

void metacheck(TypedAST::TypeTerm* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_monotype();

	metacheck(ast->m_callee.get(), tc);
	tc.m_meta_core.unify(ast->m_callee->m_meta_type, tc.meta_typefunc());

	for (auto& arg : ast->m_args) {
		metacheck(arg.get(), tc);
		tc.m_meta_core.unify(arg->m_meta_type, tc.meta_monotype());
	}
}

void metacheck(TypedAST::TypedAST* ast, TypeChecker& tc) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return void(metacheck(static_cast<TypedAST::type*>(ast), tc))

#define LITERAL(type)                                                          \
	case TypedASTTag::type:                                                    \
		return void(metacheck_literal(ast, tc))

	switch (ast->type()) {
		LITERAL(IntegerLiteral);
		LITERAL(NumberLiteral);
		LITERAL(BooleanLiteral);
		LITERAL(StringLiteral);
		LITERAL(NullLiteral);
		DISPATCH(ArrayLiteral);

		DISPATCH(Identifier);
		DISPATCH(FunctionLiteral);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(RecordAccessExpression);

		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);
	}

	std::cerr << "Unhandled case in " << __PRETTY_FUNCTION__ << " : "
	          << typed_ast_string[int(ast->type())] << "\n";
	assert(0);

#undef DISPATCH
#undef LITERAL
}

} // namespace TypeChecker
