#include "metacheck.hpp"

#include "ast.hpp"
#include "meta_unifier.hpp"

namespace TypeChecker {

static void process_type_hint(MetaUnifier& uf, AST::Declaration* ast) {
	auto typehint = ast->m_type_hint;
	if (!typehint) return;
	uf.turn_into(ast->m_meta_type, Tag::Term);
	metacheck(uf, typehint);
	uf.turn_into(typehint->m_meta_type, Tag::Mono);
}

static void process_declaration(MetaUnifier& uf, AST::Declaration* ast) {
	process_type_hint(uf, ast);
	metacheck(uf, ast->m_value);
	uf.unify(ast->m_meta_type, ast->m_value->m_meta_type);
}

// Literals

static void metacheck_scalar(MetaUnifier& uf, AST::Expr* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);
}

static void metacheck(MetaUnifier& uf, AST::ArrayLiteral* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);

	for (auto& element : ast->m_elements) {
		metacheck(uf, element);
		uf.turn_into(element->m_meta_type, Tag::Term);
	}
}

static void metacheck(MetaUnifier& uf, AST::FunctionLiteral* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);

	for (auto& arg : ast->m_args) {
		if (arg.m_type_hint) {
			metacheck(uf, arg.m_type_hint);
			uf.turn_into(arg.m_type_hint->m_meta_type, Tag::Mono);
		}
		arg.m_meta_type = uf.make_const_node(Tag::Term);
	}

	metacheck(uf, ast->m_body);
}

// Expressions

static void metacheck(MetaUnifier& uf, AST::Identifier* ast) {
	ast->m_meta_type = ast->m_declaration->m_meta_type;
}

static void metacheck(MetaUnifier& uf, AST::CallExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);

	metacheck(uf, ast->m_callee);
	uf.turn_into(ast->m_callee->m_meta_type, Tag::Term);

	for (auto& arg : ast->m_args) {
		metacheck(uf, arg);
		uf.turn_into(arg->m_meta_type, Tag::Term);
	}
}

static void metacheck(MetaUnifier& uf, AST::IndexExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);

	metacheck(uf, ast->m_callee);
	uf.turn_into(ast->m_callee->m_meta_type, Tag::Term);

	metacheck(uf, ast->m_index);
	uf.turn_into(ast->m_index->m_meta_type, Tag::Term);
}

static void metacheck(MetaUnifier& uf, AST::AccessExpression* ast) {
	metacheck(uf, ast->m_target);
	ast->m_meta_type = uf.make_dot_node(ast->m_target->m_meta_type);
}

static void metacheck(MetaUnifier& uf, AST::MatchExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);

	metacheck(uf, &ast->m_target);
	uf.turn_into(ast->m_target.m_meta_type, Tag::Term);

	if (ast->m_type_hint) {
		metacheck(uf, ast->m_type_hint);
		uf.turn_into(ast->m_type_hint->m_meta_type, Tag::Mono);
	}

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		case_data.m_declaration.m_meta_type = uf.make_const_node(Tag::Term);
		process_type_hint(uf, &case_data.m_declaration);

		metacheck(uf, case_data.m_expression);
		uf.turn_into(case_data.m_expression->m_meta_type, Tag::Term);
	}
}

static void metacheck(MetaUnifier& uf, AST::TernaryExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);

	metacheck(uf, ast->m_condition);
	uf.turn_into(ast->m_condition->m_meta_type, Tag::Term);

	metacheck(uf, ast->m_then_expr);
	uf.turn_into(ast->m_then_expr->m_meta_type, Tag::Term);

	metacheck(uf, ast->m_else_expr);
	uf.turn_into(ast->m_else_expr->m_meta_type, Tag::Term);
}

static void metacheck(MetaUnifier& uf, AST::ConstructorExpression* ast) {
	// TODO: maybe we should tag ast->m_constructor->m_meta_type
	// as being target of a constructor invokation?

	ast->m_meta_type = uf.make_const_node(Tag::Term);

	metacheck(uf, ast->m_constructor);

	for (auto& arg : ast->m_args) {
		metacheck(uf, arg);
		uf.turn_into(arg->m_meta_type, Tag::Term);
	}
}

static void metacheck(MetaUnifier& uf, AST::SequenceExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Term);
	metacheck(uf, ast->m_body);
}

// Statements

static void metacheck(MetaUnifier& uf, AST::Block* ast) {
	for (auto& child : ast->m_body)
		metacheck(uf, child);
}

static void metacheck(MetaUnifier& uf, AST::IfElseStatement* ast) {
	metacheck(uf, ast->m_condition);
	uf.turn_into(ast->m_condition->m_meta_type, Tag::Term);

	metacheck(uf, ast->m_body);

	if (ast->m_else_body)
		metacheck(uf, ast->m_else_body);
}

static void metacheck(MetaUnifier& uf, AST::WhileStatement* ast) {
	metacheck(uf, ast->m_condition);
	uf.turn_into(ast->m_condition->m_meta_type, Tag::Term);
	metacheck(uf, ast->m_body);
}

static void metacheck(MetaUnifier& uf, AST::ReturnStatement* ast) {
	metacheck(uf, ast->m_value);
	uf.turn_into(ast->m_value->m_meta_type, Tag::Term);
}

// Declarations

static void metacheck(MetaUnifier& uf, AST::Declaration* ast) {
	ast->m_meta_type = uf.make_var_node();
	process_declaration(uf, ast);
}

static void metacheck(MetaUnifier& uf, AST::Program* ast) {
	for (auto& decl : ast->m_declarations)
		decl.m_meta_type = uf.make_var_node();

	// TODO: get the declaration components
	auto const& comps = *uf.comp;
	for (auto const& comp : comps) {

		for (auto decl : comp)
			process_declaration(uf, decl);

		/*
		TODO: put this code in ct_eval? maybe it's OK here?
		for (auto decl : comp)
			if (uf.find(decl->m_meta_type).tag == Tag::Func)
				for (auto other : decl->m_references)
					if (uf.find(other->m_meta_type) == Tag::Term)
						Log::fatal("Value referenced in a type definition");
		*/
	}
}

// Type expressions

static void metacheck(MetaUnifier& uf, AST::UnionExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Func);

	for (auto& type : ast->m_types) {
		metacheck(uf, type);
		uf.turn_into(type->m_meta_type, Tag::Mono);
	}
}

static void metacheck(MetaUnifier& uf, AST::StructExpression* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Func);

	for (auto& type : ast->m_types) {
		metacheck(uf, type);
		uf.turn_into(type->m_meta_type, Tag::Mono);
	}
}

static void metacheck(MetaUnifier& uf, AST::TypeTerm* ast) {
	ast->m_meta_type = uf.make_const_node(Tag::Mono);

	metacheck(uf, ast->m_callee);
	uf.turn_into(ast->m_callee->m_meta_type, Tag::Func);

	for (auto& arg : ast->m_args) {
		metacheck(uf, arg);
		uf.turn_into(arg->m_meta_type, Tag::Mono);
	}
}

// Dispatch

void metacheck(MetaUnifier& uf, AST::AST* ast) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return metacheck(uf, static_cast<AST::type*>(ast));

#define SCALAR(type)                                                           \
	case ASTTag::type:                                                         \
		return metacheck_scalar(uf, static_cast<AST::type*>(ast));

	switch (ast->type()) {
		SCALAR(IntegerLiteral)
		SCALAR(NumberLiteral)
		SCALAR(BooleanLiteral)
		SCALAR(StringLiteral)
		SCALAR(NullLiteral)
		DISPATCH(ArrayLiteral)
		DISPATCH(FunctionLiteral)

		DISPATCH(Identifier)
		DISPATCH(CallExpression)
		DISPATCH(IndexExpression)
		DISPATCH(AccessExpression)
		DISPATCH(MatchExpression)
		DISPATCH(TernaryExpression)
		DISPATCH(ConstructorExpression)
		DISPATCH(SequenceExpression)

		DISPATCH(Block)
		DISPATCH(IfElseStatement)
		DISPATCH(WhileStatement)
		DISPATCH(ReturnStatement)

		DISPATCH(Declaration)
		DISPATCH(Program)

		DISPATCH(UnionExpression)
		DISPATCH(StructExpression)
		DISPATCH(TypeTerm)
	}
}

} // namespace TypeChecker
