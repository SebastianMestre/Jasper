#include "typecheck.hpp"

#include "../ast.hpp"
#include "../log/log.hpp"
#include "../symbol_table.hpp"
#include "typechecker.hpp"
#include "typechecker_types.hpp"

#include <cassert>
#include <unordered_set>

namespace TypeChecker {

using AST::ExprTag;
using AST::StmtTag;

static Type get_monotype_id(AST::Expr* ast) {
	switch(ast->type()) {
	case ExprTag::TypeTerm: return static_cast<AST::TypeTerm*>(ast)->m_value;
	case ExprTag::Identifier: return get_monotype_id(static_cast<AST::Identifier*>(ast)->m_declaration->m_value);
	default: assert(0);
	}
}

// this function implements 'the value restriction', a technique
// that enables type inference on mutable datatypes
static bool is_value_expression(AST::Expr* ast) {
	switch (ast->type()) {
	case ExprTag::FunctionLiteral:
	case ExprTag::Identifier:
		return true;
	default:
		return false;
	}
}

struct TypecheckHelper {
	TypecheckHelper(TypeChecker& tc)
	    : tc {tc} {}

	Type integer() { return tc.integer(); }
	Type number() { return tc.number(); }
	Type string() { return tc.string(); }
	Type boolean() { return tc.boolean(); }
	Type unit() { return tc.unit(); }

	TypeSystemCore& core() { return tc.core(); }
	std::vector<std::vector<AST::Declaration*>> const& declaration_order() const { return tc.declaration_order(); }

	Type new_var() { return tc.new_var(); }

	void declare(AST::Declaration* decl) { symbol_table.declare(decl); }
	void new_nested_scope() { symbol_table.new_nested_scope(); }
	void end_scope() { symbol_table.end_scope(); }

	void unify(Type i, Type j) { core().ll_unify(i, j); }

	Type inst_fresh(PolyId i) { return tc.core().inst_fresh(i); }

	Type new_term(TypeFunc type_function, std::vector<Type> arguments) {
		return core().new_term(type_function, std::move(arguments));
	}

	bool is_bound_to_env(VarId var) {

		for (auto& kv : symbol_table.bindings()) {
			auto decl = kv.second;

			// TODO: it's probably not ok to skip polymorphic declarations
			// but that's how it worked before, so not changing it right now.
			if (decl->m_is_polymorphic) continue;

			decl->m_value_type = core().apply_substitution(decl->m_value_type);
			auto bound_type = decl->m_value_type;
			if (core().free_vars(bound_type).count(var) != 0)
				return true;
		}

		return false;
	}


// Literals
void infer(AST::NumberLiteral* ast) {
	ast->m_value_type = number();
}

void infer(AST::IntegerLiteral* ast) {
	ast->m_value_type = integer();
}

void infer(AST::StringLiteral* ast) {
	ast->m_value_type = string();
}

void infer(AST::BooleanLiteral* ast) {
	ast->m_value_type = boolean();
}

void infer(AST::NullLiteral* ast) {
	ast->m_value_type = unit();
}

void infer(AST::ArrayLiteral* ast) {
	auto element_type = new_var();
	for (auto& element : ast->m_elements) {
		infer(element);
		unify(element_type, element->m_value_type);
	}

	ast->m_value_type = core().array(element_type);
}

// Implements [Var] rule
void infer(AST::Identifier* ast) {
	AST::Declaration* declaration = ast->m_declaration;
	assert(declaration);

	assert(declaration->m_meta_type == MetaType::Term);

	ast->m_value_type = declaration->m_is_polymorphic
		? inst_fresh(declaration->m_decl_type)
		: declaration->m_value_type;
}

// Implements [App] rule, extended for functions with multiple arguments
void infer(AST::CallExpression* ast) {
	infer(ast->m_callee);

	std::vector<Type> arg_types;
	for (auto& arg : ast->m_args) {
		infer(arg);
		arg_types.push_back(arg->m_value_type);
	}

	Type result_type = new_var();
	Type expected_callee_type = core().fun(std::move(arg_types), result_type);
	Type callee_type = ast->m_callee->m_value_type;
	unify(callee_type, expected_callee_type);

	ast->m_value_type = result_type;
}

void infer(AST::AssignmentExpression* ast) {
	infer(ast->m_target);
	infer(ast->m_value);

	unify(ast->m_target->m_value_type, ast->m_value->m_value_type);

	ast->m_value_type = ast->m_value->m_value_type;
}

// Implements [Abs] rule, extended for functions with multiple arguments
void infer(AST::FunctionLiteral* ast) {
	new_nested_scope();

	// TODO: consume return-type type-hints

	std::vector<Type> arg_types;

	for (auto& arg : ast->m_args) {
		arg.m_value_type = new_var();
		process_type_hint(&arg);
		arg_types.push_back(arg.m_value_type);
		declare(&arg);
	}

	infer(ast->m_body);

	ast->m_value_type = core().fun(std::move(arg_types), ast->m_body->m_value_type);

	end_scope();
}

void infer(AST::IndexExpression* ast) {
	infer(ast->m_callee);
	infer(ast->m_index);

	auto var = new_var();
	auto arr = core().array(var);
	unify(arr, ast->m_callee->m_value_type);

	unify(integer(), ast->m_index->m_value_type);

	ast->m_value_type = var;
}

void infer(AST::TernaryExpression* ast) {
	infer(ast->m_condition);
	unify(ast->m_condition->m_value_type, boolean());

	infer(ast->m_then_expr);
	infer(ast->m_else_expr);

	unify(ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

void infer(AST::AccessExpression* ast) {
	infer(ast->m_target);

	Type member_type = new_var();

	// constraint the target object
	Type term_type = core().ll_new_var();
	auto v = core().get_var_id(term_type);
	core().add_record_constraint(v);
	core().add_field_constraint(v, ast->m_member, member_type);
	unify(ast->m_target->m_value_type, term_type);

	ast->m_value_type = member_type;
}

void infer(AST::MatchExpression* ast) {
	infer(&ast->m_target);
	if (ast->m_type_hint) {
		unify(ast->m_target.m_value_type, get_monotype_id(ast->m_type_hint));
	}

	Type expr_ty = new_var();

	// typecheck each case of the match expression
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		case_data.m_declaration.m_value_type = new_var();
		process_type_hint(&case_data.m_declaration);

		infer(case_data.m_expression);
		unify(expr_ty, case_data.m_expression->m_value_type);
	}

	// constraint the matched-on object
	Type expected_target_ty = core().ll_new_var();
	auto v = core().get_var_id(expected_target_ty);
	core().add_variant_constraint(v);
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;
		core().add_field_constraint(v, kv.first, case_data.m_declaration.m_value_type);
	}
	unify(ast->m_target.m_value_type, expected_target_ty);

	ast->m_value_type = expr_ty;
}

void infer(AST::ConstructorExpression* ast) {

	auto& constructor = ast->m_evaluated_constructor;

	auto tf = core().type_function_of(constructor.m_type);

	// match value arguments
	if (core().is_record(tf)) {
		auto const& fields = core().fields(tf);
		assert(fields.size() == ast->m_args.size());
		for (int i = 0; i < fields.size(); ++i) {
			infer(ast->m_args[i]);
			Type field_type = core().type_of_field(tf, fields[i]);
			unify(field_type, ast->m_args[i]->m_value_type);
		}
	// match the argument type with the constructor used
	} else if (core().is_variant(tf)) {
		assert(ast->m_args.size() == 1);

		infer(ast->m_args[0]);
		InternedString id = constructor.m_id;

		Type constructor_type = core().type_of_field(tf, id);

		unify(constructor_type, ast->m_args[0]->m_value_type);
	}

	ast->m_value_type = constructor.m_type;
}

void generalize(AST::Declaration* ast) {
	assert(!ast->m_is_polymorphic);

	assert(ast->m_value);

	if (is_value_expression(ast->m_value)) {
		ast->m_is_polymorphic = true;
		ast->m_value_type = core().apply_substitution(ast->m_value_type);
		Type type = ast->m_value_type;

		std::vector<VarId> vars;
		for (VarId var : core().free_vars(type)) {
			if (!is_bound_to_env(var)) {
				vars.push_back(var);
			}
		}

		ast->m_decl_type = core().forall(std::move(vars), type);

	} else {
		// if it's not a value expression, its free vars get bound
		// to the environment instead of being generalized.
		//
		// The way variables get bound to the environment is implicitly, simply by the
		// fact that the declaration is bound to the symbol table.
	}
}

void process_type_hint(AST::Declaration* ast) {
	if (!ast->m_type_hint)
		return;

	unify(ast->m_value_type, get_monotype_id(ast->m_type_hint));
}

// typecheck the value and make the type of the decl equal
// to its type
// apply typehints if available
void process_contents(AST::Declaration* ast) {
	process_type_hint(ast);

	// it would be nicer to check this at an earlier stage
	assert(ast->m_value);
	infer(ast->m_value);
	unify(ast->m_value_type, ast->m_value->m_value_type);
}

void infer(AST::SequenceExpression* ast) {
	ast->m_value_type = new_var();
	typecheck_stmt(ast->m_body);
}

void typecheck_stmt(AST::Block* ast) {
	new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck_stmt(child);
	end_scope();
}

void typecheck_stmt(AST::IfElseStatement* ast) {
	infer(ast->m_condition);
	unify(ast->m_condition->m_value_type, boolean());

	typecheck_stmt(ast->m_body);

	if (ast->m_else_body)
		typecheck_stmt(ast->m_else_body);
}

void typecheck_stmt(AST::WhileStatement* ast) {
	infer(ast->m_condition);
	unify(ast->m_condition->m_value_type, boolean());

	typecheck_stmt(ast->m_body);
}

void typecheck_stmt(AST::ReturnStatement* ast) {
	infer(ast->m_value);

	auto mono = ast->m_value->m_value_type;
	auto seq_expr = ast->m_surrounding_seq_expr;
	unify(seq_expr->m_value_type, mono);
}

void typecheck_stmt(AST::ExpressionStatement* ast) {
	infer(ast->m_expression);
}


void typecheck_stmt(AST::Declaration* ast) {
	// put a typevar in the decl to allow recursive definitions
	ast->m_value_type = new_var();
	process_contents(ast);
	generalize(ast);
	declare(ast);
}

	void typecheck_stmt(AST::Stmt* ast) {
#define DISPATCH(type)                                                         \
	case StmtTag::type:                                                        \
		return typecheck_stmt(static_cast<AST::type*>(ast));

		// TODO: Compound literals
		switch (ast->tag()) {
			DISPATCH(Declaration);

			DISPATCH(Block);
			DISPATCH(WhileStatement);
			DISPATCH(IfElseStatement);
			DISPATCH(ReturnStatement);
			DISPATCH(ExpressionStatement);

		}

		Log::fatal() << "(internal) CST type not handled in typecheck_stmt: "
					 << AST::stmt_string[(int)ast->tag()];

#undef DISPATCH
	}

	void infer(AST::Expr* ast) {
#define DISPATCH(type)                                                         \
	case ExprTag::type:                                                        \
		return infer(static_cast<AST::type*>(ast));

#define IGNORE(type)                                                           \
	case ExprTag::type:                                                        \
		return;

		switch (ast->type()) {
			DISPATCH(NumberLiteral);
			DISPATCH(IntegerLiteral);
			DISPATCH(StringLiteral);
			DISPATCH(BooleanLiteral);
			DISPATCH(NullLiteral);
			DISPATCH(ArrayLiteral);
			DISPATCH(FunctionLiteral);

			DISPATCH(Identifier);
			DISPATCH(CallExpression);
			DISPATCH(AssignmentExpression);
			DISPATCH(IndexExpression);
			DISPATCH(TernaryExpression);
			DISPATCH(AccessExpression);
			DISPATCH(MatchExpression);
			DISPATCH(ConstructorExpression);
			DISPATCH(SequenceExpression);

			IGNORE(BuiltinTypeFunction);
		}

		Log::fatal() << "(internal) CST type not handled in infer: "
					 << AST::expr_string[(int)ast->type()];

#undef DISPATCH
#undef IGNORE
	}

	void typecheck_program(AST::Program* ast) {

		// NOTE: we don't actually do anything with `ast`: what we really care about
		// has already been precomputed and stored in `tc`. This is not the most
		// friendliest API, so maybe we could look into changing it?

		auto const& comps = declaration_order();

		// This loop implements the [Rec] rule
		for (auto const& component : comps) {

			bool terms_only = true;
			for (auto decl : component) {
				if (decl->m_meta_type != MetaType::Term) {
					terms_only = false;
				}
			}

			if (!terms_only) continue;

			new_nested_scope();

			// temporarily extend the environment with fresh type
			// variables
			for (auto decl : component) {
				decl->m_value_type = new_var();
				declare(decl);
			}

			// infer types for each declaration
			for (auto decl : component) {
				process_contents(decl);
			}

			end_scope();

			// generalize all the declarations, so that they can be
			// identified as polymorphic in the next rec-block
			for (auto decl : component) {
				generalize(decl);
			}

			// add generalized declarations to the global scope
			for (auto decl : component) {
				declare(decl);
			}

		}
	}

private:

	Frontend::SymbolTable symbol_table;
	TypeChecker& tc;


};

void typecheck_program(AST::Program* ast, TypeChecker& tc) {
	TypecheckHelper helper {tc};
	helper.typecheck_program(ast);
}

} // namespace TypeChecker
