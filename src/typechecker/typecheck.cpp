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
Type infer(AST::NumberLiteral* ast) {
	return ast->m_value_type = number();
}

Type infer(AST::IntegerLiteral* ast) {
	return ast->m_value_type = integer();
}

Type infer(AST::StringLiteral* ast) {
	return ast->m_value_type = string();
}

Type infer(AST::BooleanLiteral* ast) {
	return ast->m_value_type = boolean();
}

Type infer(AST::NullLiteral* ast) {
	return ast->m_value_type = unit();
}

Type infer(AST::ArrayLiteral* ast) {
	auto element_type = new_var();
	for (auto& element : ast->m_elements) {
		unify(element_type, infer(element));
	}

	return ast->m_value_type = core().array(element_type);
}

// Implements [Var] rule
Type infer(AST::Identifier* ast) {
	AST::Declaration* declaration = ast->m_declaration;
	assert(declaration);
	assert(declaration->m_meta_type == MetaType::Term);
	return ast->m_value_type = declaration->m_is_polymorphic
		? inst_fresh(declaration->m_decl_type)
		: declaration->m_value_type;
}

// Implements [App] rule, extended for functions with multiple arguments
Type infer(AST::CallExpression* ast) {

	Type callee_type = infer(ast->m_callee);

	std::vector<Type> arg_types;
	for (auto& arg : ast->m_args) {
		arg_types.push_back(infer(arg));
	}
	Type result_type = new_var();
	Type expected_callee_type = core().fun(std::move(arg_types), result_type);

	unify(callee_type, expected_callee_type);

	return ast->m_value_type = result_type;
}

Type infer(AST::AssignmentExpression* ast) {
	Type target_type = infer(ast->m_target);
	check(ast->m_value, target_type);
	return ast->m_value_type = target_type;
}

// Implements [Abs] rule, extended for functions with multiple arguments
Type infer(AST::FunctionLiteral* ast) {
	new_nested_scope();

	// TODO: consume return-type type-hints

	std::vector<Type> arg_types;

	for (auto& arg : ast->m_args) {
		arg.m_value_type = new_var();
		process_type_hint(&arg);
		arg_types.push_back(arg.m_value_type);
		declare(&arg);
	}

	Type body_type = infer(ast->m_body);

	end_scope();

	return ast->m_value_type = core().fun(std::move(arg_types), body_type);
}

Type infer(AST::IndexExpression* ast) {
	auto element_type = new_var();
	auto array_type = core().array(element_type);
	check(ast->m_callee, array_type);

	check(ast->m_index, integer());

	return ast->m_value_type = element_type;
}

Type infer(AST::TernaryExpression* ast) {
	check(ast->m_condition, boolean());

	auto then_type = infer(ast->m_then_expr);
	auto else_type = infer(ast->m_else_expr);

	unify(then_type, else_type);

	return ast->m_value_type = then_type;
}

Type infer(AST::AccessExpression* ast) {
	// constraint the target object
	Type member_type = new_var();
	Type term_type = new_var();
	auto v = core().get_var_id(term_type);
	core().add_record_constraint(v);
	core().add_field_constraint(v, ast->m_member, member_type);
	check(ast->m_target, term_type);

	return ast->m_value_type = member_type;
}

Type infer(AST::MatchExpression* ast) {
	Type target_type;
	if (ast->m_type_hint) {
		target_type = get_monotype_id(ast->m_type_hint);
		check(&ast->m_target, target_type);
	} else {
		target_type = infer(&ast->m_target);
	}

	Type expr_ty = new_var();

	// typecheck each case of the match expression
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		case_data.m_declaration.m_value_type = new_var();
		process_type_hint(&case_data.m_declaration);

		Type case_result_type = infer(case_data.m_expression);
		unify(expr_ty, case_result_type);
	}

	// constraint the matched-on object
	Type expected_target_ty = core().ll_new_var();
	auto v = core().get_var_id(expected_target_ty);
	core().add_variant_constraint(v);
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;
		core().add_field_constraint(v, kv.first, case_data.m_declaration.m_value_type);
	}
	unify(target_type, expected_target_ty);

	return ast->m_value_type = expr_ty;
}

Type infer(AST::ConstructorExpression* ast) {

	auto& constructor = ast->m_evaluated_constructor;

	auto tf = core().type_function_of(constructor.m_type);

	// match value arguments
	if (core().is_record(tf)) {
		auto const& fields = core().fields(tf);
		assert(fields.size() == ast->m_args.size());
		for (int i = 0; i < fields.size(); ++i) {
			Type field_type = core().type_of_field(tf, fields[i]);
			check(ast->m_args[i], field_type);
		}
	// match the argument type with the constructor used
	} else if (core().is_variant(tf)) {
		assert(ast->m_args.size() == 1);
		InternedString id = constructor.m_id;
		Type constructor_type = core().type_of_field(tf, id);
		check(ast->m_args[0], constructor_type);
	}

	return ast->m_value_type = constructor.m_type;
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
	check(ast->m_value, ast->m_value_type);
}

Type infer(AST::SequenceExpression* ast) {
	ast->m_value_type = new_var();
	typecheck_stmt(ast->m_body);
	return ast->m_value_type;
}

void typecheck_stmt(AST::Block* ast) {
	new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck_stmt(child);
	end_scope();
}

void typecheck_stmt(AST::IfElseStatement* ast) {
	check(ast->m_condition, boolean());
	typecheck_stmt(ast->m_body);
	if (ast->m_else_body)
		typecheck_stmt(ast->m_else_body);
}

void typecheck_stmt(AST::WhileStatement* ast) {
	check(ast->m_condition, boolean());
	typecheck_stmt(ast->m_body);
}

void typecheck_stmt(AST::ReturnStatement* ast) {
	auto seq_expr = ast->m_surrounding_seq_expr;
	check(ast->m_value, seq_expr->m_value_type);
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

	void check(AST::Expr* ast, Type expected_type) {
		auto inferred_type = infer(ast);
		unify(expected_type, inferred_type);
	}

	Type infer(AST::Expr* ast) {
#define DISPATCH(type)                                                         \
	case ExprTag::type:                                                        \
		return infer(static_cast<AST::type*>(ast));

#define IGNORE(type)                                                           \
	case ExprTag::type:                                                        \
		assert(0); break;

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
