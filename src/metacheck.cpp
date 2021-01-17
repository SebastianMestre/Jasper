#include "metacheck.hpp"

#include "./log/log.hpp"
#include "typechecker.hpp"
#include "typed_ast.hpp"

#include <sstream>
#include <cassert>

namespace TypeChecker {

static void process_declaration_type_hint(TypedAST::Declaration* ast, TypeChecker& tc);

void unsafe_assign_meta_type(MetaTypeId& target, MetaTypeId value) {
	target = value;
}

void assign_meta_type(MetaTypeId& target, MetaTypeId value, TypeChecker& tc) {
	if (target == -1) {
		target = value;
	} else {
		tc.m_core.m_meta_core.unify(target, value);
	}
}

// literals

void metacheck_literal(TypedAST::TypedAST* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);
}

void metacheck(TypedAST::ArrayLiteral* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	for (auto& element : ast->m_elements) {
		metacheck(element, tc);
		tc.m_core.m_meta_core.unify(element->m_meta_type, tc.meta_value());
	}
}

// expressions

void metacheck(TypedAST::Identifier* ast, TypeChecker& tc) {
	assert(ast->m_declaration);
	assign_meta_type(ast->m_meta_type, ast->m_declaration->m_meta_type, tc);
}

void metacheck(TypedAST::FunctionLiteral* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	for (auto& arg : ast->m_args) {
		if (arg.m_type_hint) {
			metacheck(arg.m_type_hint, tc);
			assign_meta_type(arg.m_type_hint->m_meta_type, tc.meta_monotype(), tc);
		}
		assign_meta_type(arg.m_meta_type, tc.meta_value(), tc);
	}

	metacheck(ast->m_body, tc);
}

void metacheck(TypedAST::CallExpression* ast, TypeChecker& tc) {
	// TODO? maybe we would like to support compile time functions that return types eventually?
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	metacheck(ast->m_callee, tc);
	tc.m_core.m_meta_core.unify(ast->m_callee->m_meta_type, tc.meta_value());

	for (auto& arg : ast->m_args) {
		metacheck(arg, tc);
		tc.m_core.m_meta_core.unify(arg->m_meta_type, tc.meta_value());
	}
}

void metacheck(TypedAST::IndexExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	metacheck(ast->m_callee, tc);
	tc.m_core.m_meta_core.unify(ast->m_callee->m_meta_type, tc.meta_value());

	metacheck(ast->m_index, tc);
	tc.m_core.m_meta_core.unify(ast->m_index->m_meta_type, tc.meta_value());
}

void metacheck(TypedAST::IfElseExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	metacheck(ast->m_condition, tc);
	tc.m_core.m_meta_core.unify(ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_then_expr, tc);
	tc.m_core.m_meta_core.unify(ast->m_then_expr->m_meta_type, tc.meta_value());

	metacheck(ast->m_else_expr, tc);
	tc.m_core.m_meta_core.unify(ast->m_else_expr->m_meta_type, tc.meta_value());
}

void metacheck(TypedAST::AccessExpression* ast, TypeChecker& tc) {
	// first time through metacheck
	if (ast->m_meta_type == -1)
		ast->m_meta_type = tc.new_meta_var();

	metacheck(ast->m_record, tc);
	MetaTypeId metatype = tc.m_core.m_meta_core.find(ast->m_record->m_meta_type);

	if (tc.m_core.m_meta_core.is_var(metatype)) {
		if (tc.m_in_last_metacheck_pass) {
			// The only way to get a var is if it's in the same SCC
			// (because we process in topological order). If it's in the
			// same SCC, then there are cyclic references. If there are
			// cyclic references, then either both are type-ey things, or
			// both are values. Both being type-ey things makes no sense,
			// so they must both be values.
			tc.m_core.m_meta_core.unify(ast->m_meta_type, tc.meta_value());
		}
	} else  {
		// TODO: we would like to support static records with
		// typefunc members in the future
		auto correct_metatype = -1;
		if (metatype == tc.meta_monotype())
			correct_metatype = tc.meta_constructor();
		else if (metatype == tc.meta_value())
			correct_metatype = tc.meta_value();
		else
			assert(0);

		tc.m_core.m_meta_core.unify(ast->m_meta_type, correct_metatype);
	}
}

void metacheck(TypedAST::MatchExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	metacheck(&ast->m_matchee, tc);
	tc.m_core.m_meta_core.unify(ast->m_matchee.m_meta_type, tc.meta_value());

	if (ast->m_type_hint) {
		metacheck(ast->m_type_hint, tc);
		tc.m_core.m_meta_core.unify(
		    ast->m_type_hint->m_meta_type, tc.meta_monotype());
	}

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		assign_meta_type(case_data.m_declaration.m_meta_type, tc.meta_value(), tc);
		process_declaration_type_hint(&case_data.m_declaration, tc);

		metacheck(case_data.m_expression, tc);
		tc.m_core.m_meta_core.unify(case_data.m_expression->m_meta_type, tc.meta_value());
	}
}

void metacheck(TypedAST::ConstructorExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);

	metacheck(ast->m_constructor, tc);

	for (auto& arg : ast->m_args) {
		metacheck(arg, tc);
		tc.m_core.m_meta_core.unify(arg->m_meta_type, tc.meta_value());
	}
}

void metacheck(TypedAST::SequenceExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_value(), tc);
	metacheck(ast->m_body, tc);
}

// statements

void metacheck(TypedAST::Block* ast, TypeChecker& tc) {
	for (auto& child : ast->m_body)
		metacheck(child, tc);
}

void metacheck(TypedAST::IfElseStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_condition, tc);
	tc.m_core.m_meta_core.unify(ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_body, tc);

	if (ast->m_else_body)
		metacheck(ast->m_else_body, tc);
}

void metacheck(TypedAST::ForStatement* ast, TypeChecker& tc) {
	metacheck(&ast->m_declaration, tc);
	metacheck(ast->m_condition, tc);
	tc.m_core.m_meta_core.unify(
	    ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_action, tc);
	metacheck(ast->m_body, tc);
}

void metacheck(TypedAST::WhileStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_condition, tc);
	tc.m_core.m_meta_core.unify(
	    ast->m_condition->m_meta_type, tc.meta_value());

	metacheck(ast->m_body, tc);
}

void metacheck(TypedAST::ReturnStatement* ast, TypeChecker& tc) {
	metacheck(ast->m_value, tc);
}

// declarations

static void process_declaration_type_hint(TypedAST::Declaration* ast, TypeChecker& tc) {
	auto* type_hint = ast->m_type_hint;
	if (type_hint) {
		metacheck(type_hint, tc);
		tc.m_core.m_meta_core.unify(type_hint->m_meta_type, tc.meta_monotype());
		tc.m_core.m_meta_core.unify(ast->m_meta_type, tc.meta_value());
	}
}

void process_declaration(TypedAST::Declaration* ast, TypeChecker& tc) {
	process_declaration_type_hint(ast, tc);
	metacheck(ast->m_value, tc);
	tc.m_core.m_meta_core.unify(ast->m_meta_type, ast->m_value->m_meta_type);
}

void metacheck(TypedAST::Declaration* ast, TypeChecker& tc) {
	if (ast->m_meta_type == -1)
		ast->m_meta_type = tc.new_meta_var();

	process_declaration(ast, tc);
}

void metacheck(TypedAST::DeclarationList* ast, TypeChecker& tc) {
	for (auto& decl : ast->m_declarations)
		// this is OK because we haven't done any metachecking yet.
		unsafe_assign_meta_type(decl.m_meta_type, tc.new_meta_var());

	auto const& comps = tc.m_env.declaration_components;
	for (auto const& comp : comps) {

		// We do a first pass to get some information off of the ASTs.
		// After that, most things should be inferable during the second pass,
		// but we will set a flag to activate a special behavior in case some
		// things aren't.
		for (auto decl : comp)
			process_declaration(decl, tc);

		tc.m_in_last_metacheck_pass = true;

		for (auto decl : comp)
			process_declaration(decl, tc);

		tc.m_in_last_metacheck_pass = false;

		for (auto decl : comp)
			if (tc.m_core.m_meta_core.find(decl->m_meta_type) == tc.meta_typefunc())
				for (auto other : decl->m_references)
					if (tc.m_core.m_meta_core.find(other->m_meta_type) ==
					    tc.meta_value())
						Log::fatal("Value referenced in a type definition");
	}
}

void metacheck(TypedAST::UnionExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_typefunc(), tc);

	for (auto& type : ast->m_types) {
		metacheck(type, tc);
		tc.m_core.m_meta_core.unify(type->m_meta_type, tc.meta_monotype());
	}
}

void metacheck(TypedAST::StructExpression* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_typefunc(), tc);

	for (auto& type : ast->m_types) {
		metacheck(type, tc);
		tc.m_core.m_meta_core.unify(type->m_meta_type, tc.meta_monotype());
	}
}

void metacheck(TypedAST::TypeTerm* ast, TypeChecker& tc) {
	assign_meta_type(ast->m_meta_type, tc.meta_monotype(), tc);

	metacheck(ast->m_callee, tc);
	tc.m_core.m_meta_core.unify(ast->m_callee->m_meta_type, tc.meta_typefunc());

	for (auto& arg : ast->m_args) {
		metacheck(arg, tc);
		tc.m_core.m_meta_core.unify(arg->m_meta_type, tc.meta_monotype());
	}
}

void metacheck(TypedAST::TypedAST* ast, TypeChecker& tc) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return void(metacheck(static_cast<TypedAST::type*>(ast), tc))

#define LITERAL(type)                                                          \
	case TypedASTTag::type:                                                    \
		return void(metacheck_literal(ast, tc))

#define REJECT(type)                                                          \
	case TypedASTTag::type:                                                    \
		assert(0);

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
		DISPATCH(IfElseExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(UnionExpression);
		DISPATCH(StructExpression);
		DISPATCH(TypeTerm);

		REJECT(TypeFunctionHandle);
		REJECT(MonoTypeHandle);
		REJECT(Constructor);
	}

	Log::FatalStream() << "(internal) Unhandled case in metacheck : "
	                   << typed_ast_string[int(ast->type())];

#undef DISPATCH
#undef LITERAL
}

} // namespace TypeChecker
