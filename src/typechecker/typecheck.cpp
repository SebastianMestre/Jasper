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

struct TypecheckHelper {
	TypecheckHelper(TypeChecker& tc)
	    : tc {tc} {}

	PolyId generalize(Type mono);

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

private:

	Frontend::SymbolTable symbol_table;
	TypeChecker& tc;

	bool is_bound_to_env(VarId var) {

		for (auto& kv : symbol_table.bindings()) {
			auto decl = kv.second;

			// TODO: it's probably not ok to skip polymorphic declarations
			// but that's how it worked before, so not changing it right now.
			if (decl->m_is_polymorphic) continue;

			auto bound_type = decl->m_value_type;
			if (core().free_vars(bound_type).count(var) != 0)
				return true;
		}

		return false;
	}

};


// quantifies all free variables in the given monotype
PolyId TypecheckHelper::generalize(Type mono) {

	std::vector<VarId> vars;
	for (VarId var : core().free_vars(mono)) {
		if (!is_bound_to_env(var)) {
			vars.push_back(var);
		}
	}

	return core().forall(std::move(vars), mono);
}

static void infer(AST::Expr* ast, TypecheckHelper& tc);

static void typecheck_stmt(AST::Stmt* ast, TypecheckHelper& tc);

static void process_type_hint(AST::Declaration* ast, TypecheckHelper& tc);

static Type get_monotype_id(AST::Expr* ast) {
	switch(ast->type()) {
	case ExprTag::TypeTerm: return static_cast<AST::TypeTerm*>(ast)->m_value;
	case ExprTag::Identifier: return get_monotype_id(static_cast<AST::Identifier*>(ast)->m_declaration->m_value);
	default: assert(0);
	}
}

// Literals
static void infer(AST::NumberLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.number();
}

static void infer(AST::IntegerLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.integer();
}

static void infer(AST::StringLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.string();
}

static void infer(AST::BooleanLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.boolean();
}

static void infer(AST::NullLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.unit();
}

static void infer(AST::ArrayLiteral* ast, TypecheckHelper& tc) {
	auto element_type = tc.new_var();
	for (auto& element : ast->m_elements) {
		infer(element, tc);
		tc.unify(element_type, element->m_value_type);
	}

	ast->m_value_type = tc.core().array(element_type);
}

// Implements [Var] rule
static void infer(AST::Identifier* ast, TypecheckHelper& tc) {
	AST::Declaration* declaration = ast->m_declaration;
	assert(declaration);

	assert(declaration->m_meta_type == MetaType::Term);

	ast->m_value_type = declaration->m_is_polymorphic
		? tc.inst_fresh(declaration->m_decl_type)
		: declaration->m_value_type;
}

// Implements [App] rule, extended for functions with multiple arguments
static void infer(AST::CallExpression* ast, TypecheckHelper& tc) {
	infer(ast->m_callee, tc);

	std::vector<Type> arg_types;
	for (auto& arg : ast->m_args) {
		infer(arg, tc);
		arg_types.push_back(arg->m_value_type);
	}

	Type result_type = tc.new_var();
	Type expected_callee_type = tc.core().fun(std::move(arg_types), result_type);
	Type callee_type = ast->m_callee->m_value_type;
	tc.unify(callee_type, expected_callee_type);

	ast->m_value_type = result_type;
}


static void infer(AST::AssignmentExpression* ast, TypecheckHelper& tc) {
	infer(ast->m_target, tc);
	infer(ast->m_value, tc);

	tc.unify(ast->m_target->m_value_type, ast->m_value->m_value_type);

	ast->m_value_type = ast->m_value->m_value_type;
}

// Implements [Abs] rule, extended for functions with multiple arguments
static void infer(AST::FunctionLiteral* ast, TypecheckHelper& tc) {
	tc.new_nested_scope();

	// TODO: consume return-type type-hints

	std::vector<Type> arg_types;

	for (auto& arg : ast->m_args) {
		arg.m_value_type = tc.new_var();
		process_type_hint(&arg, tc);
		arg_types.push_back(arg.m_value_type);
		tc.declare(&arg);
	}

	infer(ast->m_body, tc);

	ast->m_value_type = tc.core().fun(std::move(arg_types), ast->m_body->m_value_type);

	tc.end_scope();
}

static void infer(AST::IndexExpression* ast, TypecheckHelper& tc) {
	infer(ast->m_callee, tc);
	infer(ast->m_index, tc);

	auto var = tc.new_var();
	auto arr = tc.core().array(var);
	tc.unify(arr, ast->m_callee->m_value_type);

	tc.unify(tc.integer(), ast->m_index->m_value_type);

	ast->m_value_type = var;
}

static void infer(AST::TernaryExpression* ast, TypecheckHelper& tc) {
	infer(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.boolean());

	infer(ast->m_then_expr, tc);
	infer(ast->m_else_expr, tc);

	tc.unify(ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

static void infer(AST::AccessExpression* ast, TypecheckHelper& tc) {
	infer(ast->m_target, tc);

	Type member_type = tc.new_var();

	// constraint the target object
	Type term_type = tc.core().ll_new_var();
	auto v = tc.core().get_var_id(term_type);
	tc.core().add_record_constraint(v);
	tc.core().add_field_constraint(v, ast->m_member, member_type);
	tc.unify(ast->m_target->m_value_type, term_type);

	ast->m_value_type = member_type;
}

static void infer(AST::MatchExpression* ast, TypecheckHelper& tc) {
	infer(&ast->m_target, tc);
	if (ast->m_type_hint) {
		tc.unify(ast->m_target.m_value_type, get_monotype_id(ast->m_type_hint));
	}

	Type expr_ty = tc.new_var();

	// typecheck each case of the match expression
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		case_data.m_declaration.m_value_type = tc.new_var();
		process_type_hint(&case_data.m_declaration, tc);

		infer(case_data.m_expression, tc);
		tc.unify(expr_ty, case_data.m_expression->m_value_type);
	}

	// constraint the matched-on object
	Type expected_target_ty = tc.core().ll_new_var();
	auto v = tc.core().get_var_id(expected_target_ty);
	tc.core().add_variant_constraint(v);
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;
		tc.core().add_field_constraint(v, kv.first, case_data.m_declaration.m_value_type);
	}
	tc.unify(ast->m_target.m_value_type, expected_target_ty);

	ast->m_value_type = expr_ty;
}

static void infer(AST::ConstructorExpression* ast, TypecheckHelper& tc) {

	auto& constructor = ast->m_evaluated_constructor;

	auto tf = tc.core().type_function_of(constructor.m_type);

	// match value arguments
	if (tc.core().is_record(tf)) {
		auto const& fields = tc.core().fields(tf);
		assert(fields.size() == ast->m_args.size());
		for (int i = 0; i < fields.size(); ++i) {
			infer(ast->m_args[i], tc);
			Type field_type = tc.core().type_of_field(tf, fields[i]);
			tc.unify(field_type, ast->m_args[i]->m_value_type);
		}
	// match the argument type with the constructor used
	} else if (tc.core().is_variant(tf)) {
		assert(ast->m_args.size() == 1);

		infer(ast->m_args[0], tc);
		InternedString id = constructor.m_id;

		Type constructor_type = tc.core().type_of_field(tf, id);

		tc.unify(constructor_type, ast->m_args[0]->m_value_type);
	}

	ast->m_value_type = constructor.m_type;
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

void generalize(AST::Declaration* ast, TypecheckHelper& tc) {
	assert(!ast->m_is_polymorphic);

	assert(ast->m_value);

	if (is_value_expression(ast->m_value)) {
		ast->m_is_polymorphic = true;
		ast->m_decl_type = tc.generalize(ast->m_value_type);
	} else {
		// if it's not a value expression, its free vars get bound
		// to the environment instead of being generalized.
		//
		// The way variables get bound to the environment is implicitly, simply by the
		// fact that the declaration is bound to the symbol table.
	}
}

static void process_type_hint(AST::Declaration* ast, TypecheckHelper& tc) {
	if (!ast->m_type_hint)
		return;

	tc.unify(ast->m_value_type, get_monotype_id(ast->m_type_hint));
}

// typecheck the value and make the type of the decl equal
// to its type
// apply typehints if available
void process_contents(AST::Declaration* ast, TypecheckHelper& tc) {
	process_type_hint(ast, tc);

	// it would be nicer to check this at an earlier stage
	assert(ast->m_value);
	infer(ast->m_value, tc);
	tc.unify(ast->m_value_type, ast->m_value->m_value_type);
}

static void infer(AST::SequenceExpression* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.new_var();
	typecheck_stmt(ast->m_body, tc);
}

static void typecheck_stmt(AST::Block* ast, TypecheckHelper& tc) {
	tc.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck_stmt(child, tc);
	tc.end_scope();
}

static void typecheck_stmt(AST::IfElseStatement* ast, TypecheckHelper& tc) {
	infer(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.boolean());

	typecheck_stmt(ast->m_body, tc);

	if (ast->m_else_body)
		typecheck_stmt(ast->m_else_body, tc);
}

static void typecheck_stmt(AST::WhileStatement* ast, TypecheckHelper& tc) {
	// TODO: Why do while statements create a new scope?
	tc.new_nested_scope();
	infer(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.boolean());

	typecheck_stmt(ast->m_body, tc);
	tc.end_scope();
}

static void typecheck_stmt(AST::ReturnStatement* ast, TypecheckHelper& tc) {
	infer(ast->m_value, tc);

	auto mono = ast->m_value->m_value_type;
	auto seq_expr = ast->m_surrounding_seq_expr;
	tc.unify(seq_expr->m_value_type, mono);
}

static void typecheck_stmt(AST::ExpressionStatement* ast, TypecheckHelper& tc) {
	infer(ast->m_expression, tc);
}


static void typecheck_stmt(AST::Declaration* ast, TypecheckHelper& tc) {
	// put a typevar in the decl to allow recursive definitions
	ast->m_value_type = tc.new_var();
	process_contents(ast, tc);
	generalize(ast, tc);
	tc.declare(ast);
}

static void typecheck_stmt(AST::Stmt* ast, TypecheckHelper& tc) {
#define DISPATCH(type)                                                         \
	case StmtTag::type:                                                        \
		return typecheck_stmt(static_cast<AST::type*>(ast), tc);

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

static void infer(AST::Expr* ast, TypecheckHelper& tc) {
#define DISPATCH(type)                                                         \
	case ExprTag::type:                                                        \
		return infer(static_cast<AST::type*>(ast), tc);

#define IGNORE(type)                                                           \
	case ExprTag::type:                                                        \
		return;

	// TODO: Compound literals
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

void typecheck_program(AST::Program* ast, TypeChecker& tc_) {
	TypecheckHelper tc = {tc_};

	// NOTE: we don't actually do anything with `ast`: what we really care about
	// has already been precomputed and stored in `tc`. This is not the most
	// friendliest API, so maybe we could look into changing it?

	auto const& comps = tc.declaration_order();

	// This loop implements the [Rec] rule
	for (auto const& component : comps) {

		bool terms_only = true;
		for (auto decl : component) {
			if (decl->m_meta_type != MetaType::Term) {
				terms_only = false;
			}
		}

		if (!terms_only) continue;

		tc.new_nested_scope();

		// temporarily extend the environment with fresh type
		// variables
		for (auto decl : component) {
			decl->m_value_type = tc.new_var();
			tc.declare(decl);
		}

		// infer types for each declaration
		for (auto decl : component) {
			process_contents(decl, tc);
		}

		tc.end_scope();

		// generalize all the declarations, so that they can be
		// identified as polymorphic in the next rec-block
		for (auto decl : component) {
			generalize(decl, tc);
		}

		// add generalized declarations to the global scope
		for (auto decl : component) {
			tc.declare(decl);
		}

	}
}

} // namespace TypeChecker
