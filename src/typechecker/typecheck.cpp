#include "typecheck.hpp"

#include "../ast.hpp"
#include "../log/log.hpp"
#include "compile_time_environment.hpp"
#include "typechecker.hpp"
#include "typechecker_types.hpp"

#include <cassert>
#include <unordered_set>

namespace TypeChecker {

struct TypecheckHelper {
	TypecheckHelper(TypeChecker& tc)
	    : tc {tc} {}

	void bind_free_vars(MonoId mono);
	PolyId generalize(MonoId mono);

	MonoId mono_int() { return tc.mono_int(); }
	MonoId mono_float() { return tc.mono_float(); }
	MonoId mono_string() { return tc.mono_string(); }
	MonoId mono_boolean() { return tc.mono_boolean(); }
	MonoId mono_unit() { return tc.mono_unit(); }

	TypeSystemCore& core() { return tc.core(); }
	std::vector<std::vector<AST::Declaration*>> const& declaration_order() const { return tc.declaration_order(); }

	MonoId new_hidden_var() { return tc.new_hidden_var(); }
	MonoId new_var() { return tc.new_var(); }

	void new_nested_scope() { tc.m_env.new_nested_scope(); }
	void end_scope() { tc.m_env.end_scope(); }

	void unify(MonoId i, MonoId j) { core().ll_unify(i, j); }

	bool is_type(MetaTypeId i) { return meta_type_is(i, Tag::Func) || meta_type_is(i, Tag::Mono); }
	bool is_term(MetaTypeId i) { return meta_type_is(i, Tag::Term); }

	MonoId inst_fresh(PolyId i) { return tc.core().inst_fresh(i); }

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> arguments,
	    char const* debug_data = nullptr) {
		return core().new_term(type_function, std::move(arguments), debug_data);
	}


	MonoId make_dummy_variant_type(std::unordered_map<InternedString, MonoId> structure) {
		return core().new_dummy_for_typecheck1(std::move(structure));
	}

	MonoId make_dummy_record_type(std::unordered_map<InternedString, MonoId> structure) {
		return core().new_dummy_for_typecheck2(std::move(structure));
	}

	MonoId make_function_type(std::vector<MonoId> args_types, int return_type) {
		// return type goes last in a function type
		args_types.push_back(return_type);
		return new_term(BuiltinType::Function, std::move(args_types));
	}

private:
	bool meta_type_is(MetaTypeId i, Tag t) {
		i = get_resolved_meta_type(i);
		return core().m_meta_core.is(i, t);
	}

	MetaTypeId get_resolved_meta_type(MetaTypeId i) {
		return core().m_meta_core.eval(i);
	}

	Frontend::CompileTimeEnvironment& env() { return tc.env(); }

	TypeChecker& tc;

	bool is_bound_to_env(MonoId var) {

		for (auto& scope : env().scopes())
			for (auto bound_type : scope.m_type_vars)
				if (free_vars_of(bound_type).count(var) != 0)
					return true;

		return false;
	}

	void bind_to_env(MonoId var) {
		env().bind_to_current_scope(var);
	}

	std::unordered_set<MonoId> free_vars_of(MonoId mono) {
		std::unordered_set<MonoId> free_vars;
		core().gather_free_vars(mono, free_vars);
		return free_vars;
	}
};


void TypecheckHelper::bind_free_vars(MonoId mono) {
	for (MonoId var : free_vars_of(mono)) {
		if (!is_bound_to_env(var))
			bind_to_env(var);
	}
}

// qualifies all free variables in the given monotype
PolyId TypecheckHelper::generalize(MonoId mono) {

	std::vector<MonoId> new_vars;
	std::unordered_map<MonoId, MonoId> mapping;
	for (MonoId var : free_vars_of(mono)) {
		if (!is_bound_to_env(var)) {
			auto fresh_var = new_hidden_var();
			new_vars.push_back(fresh_var);
			mapping[var] = fresh_var;
		}
	}

	MonoId base = core().inst_impl(mono, mapping);

	return core().new_poly(base, std::move(new_vars));
}

void typecheck(AST::AST* ast, TypecheckHelper& tc);

void typecheck_program(AST::AST* ast, TypecheckHelper& tc);

static void typecheck_stmt(AST::Stmt* ast, TypecheckHelper& tc);

void typecheck(AST::AST* ast, TypeChecker& tc) {
	TypecheckHelper f = {tc};
	typecheck_program(ast, f);
}

static void process_type_hint(AST::Declaration* ast, TypecheckHelper& tc);

static MonoId get_monotype_id(AST::Expr* ast) {
	switch(ast->type()) {
	case ASTTag::TypeTerm:
		return static_cast<AST::TypeTerm*>(ast)->m_value;
	default: assert(0);
	}
}

// Literals
void typecheck(AST::NumberLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.mono_float();
}

void typecheck(AST::IntegerLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.mono_int();
}

void typecheck(AST::StringLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.mono_string();
}

void typecheck(AST::BooleanLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.mono_boolean();
}

void typecheck(AST::NullLiteral* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.mono_unit();
}

void typecheck(AST::ArrayLiteral* ast, TypecheckHelper& tc) {
	auto element_type = tc.new_hidden_var();
	for (auto& element : ast->m_elements) {
		typecheck(element, tc);
		tc.unify(element_type, element->m_value_type);
	}

	ast->m_value_type =
	    tc.new_term(BuiltinType::Array, {element_type}, "Array Literal");
}

// Implements [Var] rule
void typecheck(AST::Identifier* ast, TypecheckHelper& tc) {
	AST::Declaration* declaration = ast->m_declaration;
	assert(declaration);

	assert(tc.is_term(declaration->m_meta_type));

	ast->m_value_type = declaration->m_is_polymorphic
		? tc.inst_fresh(declaration->m_decl_type)
		: declaration->m_value_type;
}

// Implements [App] rule, extended for functions with multiple arguments
void typecheck(AST::CallExpression* ast, TypecheckHelper& tc) {
	typecheck(ast->m_callee, tc);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args) {
		typecheck(arg, tc);
		arg_types.push_back(arg->m_value_type);
	}

	MonoId result_type = tc.new_hidden_var();
	MonoId expected_callee_type = tc.make_function_type(std::move(arg_types), result_type);
	MonoId callee_type = ast->m_callee->m_value_type;
	tc.unify(callee_type, expected_callee_type);

	ast->m_value_type = result_type;
}

// Implements [Abs] rule, extended for functions with multiple arguments
void typecheck(AST::FunctionLiteral* ast, TypecheckHelper& tc) {
	tc.new_nested_scope();

	// TODO: consume return-type type-hints

	std::vector<MonoId> arg_types;

	for (auto& arg : ast->m_args) {
		arg.m_value_type = tc.new_var();
		process_type_hint(&arg, tc);
		arg_types.push_back(arg.m_value_type);
	}

	typecheck(ast->m_body, tc);

	ast->m_value_type = tc.make_function_type(std::move(arg_types), ast->m_body->m_value_type);

	tc.end_scope();
}

void typecheck(AST::IndexExpression* ast, TypecheckHelper& tc) {
	typecheck(ast->m_callee, tc);
	typecheck(ast->m_index, tc);

	auto var = tc.new_hidden_var();
	auto arr = tc.new_term(BuiltinType::Array, {var});
	tc.unify(arr, ast->m_callee->m_value_type);

	tc.unify(tc.mono_int(), ast->m_index->m_value_type);

	ast->m_value_type = var;
}

void typecheck(AST::TernaryExpression* ast, TypecheckHelper& tc) {
	typecheck(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_then_expr, tc);
	typecheck(ast->m_else_expr, tc);

	tc.unify(ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

void typecheck(AST::AccessExpression* ast, TypecheckHelper& tc) {
	typecheck(ast->m_target, tc);

	// should this be a hidden type var?
	MonoId member_type = tc.new_var();
	ast->m_value_type = member_type;

	MonoId term_type = tc.make_dummy_record_type({{ast->m_member, member_type}});

	tc.unify(ast->m_target->m_value_type, term_type);
}

void typecheck(AST::MatchExpression* ast, TypecheckHelper& tc) {
	typecheck(&ast->m_target, tc);
	if (ast->m_type_hint) {
		tc.unify(ast->m_target.m_value_type, get_monotype_id(ast->m_type_hint));
	}

	ast->m_value_type = tc.new_var();

	std::unordered_map<InternedString, MonoId> dummy_structure;
	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		// deduce type of each declaration
		// NOTE(SMestre): this tc.new_var() worries me. Since we are putting it
		// a typefunc, it should not ever get generalized. But we don't really
		// do anything to prevent it.
		case_data.m_declaration.m_value_type = tc.new_var();
		process_type_hint(&case_data.m_declaration, tc);

		// unify type of match with type of cases
		typecheck(case_data.m_expression, tc);
		tc.unify(ast->m_value_type, case_data.m_expression->m_value_type);

		// get the structure of the match expression for a dummy
		dummy_structure[kv.first] = case_data.m_declaration.m_value_type;
	}

	MonoId term_type = tc.make_dummy_variant_type(std::move(dummy_structure));
	tc.unify(ast->m_target.m_value_type, term_type);
}

void typecheck(AST::ConstructorExpression* ast, TypecheckHelper& tc) {
	typecheck(ast->m_constructor, tc);

	auto constructor = static_cast<AST::Constructor*>(ast->m_constructor);
	assert(constructor->type() == ASTTag::Constructor);

	TypeFunctionData& tf_data = tc.core().type_function_data_of(constructor->m_mono);

	// match value arguments
	if (tf_data.tag == TypeFunctionTag::Record) {
		assert(tf_data.fields.size() == ast->m_args.size());
		for (int i = 0; i < ast->m_args.size(); ++i) {
			typecheck(ast->m_args[i], tc);
			MonoId field_type = tf_data.structure[tf_data.fields[i]];
			tc.unify(field_type, ast->m_args[i]->m_value_type);
		}
	// match the argument type with the constructor used
	} else if (tf_data.tag == TypeFunctionTag::Variant) {
		assert(ast->m_args.size() == 1);

		typecheck(ast->m_args[0], tc);
		InternedString id = constructor->m_id;
		MonoId constructor_type = tf_data.structure[id];

		tc.unify(constructor_type, ast->m_args[0]->m_value_type);
	}

	ast->m_value_type = constructor->m_mono;
}

// this function implements 'the value restriction', a technique
// that enables type inference on mutable datatypes
static bool is_value_expression(AST::AST* ast) {
	switch (ast->type()) {
	case ASTTag::FunctionLiteral:
	case ASTTag::Identifier:
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
		// to the environment instead of being generalized
		tc.bind_free_vars(ast->m_value_type);
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
	typecheck(ast->m_value, tc);
	tc.unify(ast->m_value_type, ast->m_value->m_value_type);
}

static void typecheck(AST::SequenceExpression* ast, TypecheckHelper& tc) {
	ast->m_value_type = tc.new_hidden_var();
	typecheck_stmt(ast->m_body, tc);
}

static void typecheck_stmt(AST::Block* ast, TypecheckHelper& tc) {
	tc.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck_stmt(child, tc);
	tc.end_scope();
}

static void typecheck_stmt(AST::IfElseStatement* ast, TypecheckHelper& tc) {
	typecheck(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck_stmt(ast->m_body, tc);

	if (ast->m_else_body)
		typecheck_stmt(ast->m_else_body, tc);
}

static void typecheck_stmt(AST::WhileStatement* ast, TypecheckHelper& tc) {
	// TODO: Why do while statements create a new scope?
	tc.new_nested_scope();
	typecheck(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck_stmt(ast->m_body, tc);
	tc.end_scope();
}

static void typecheck_stmt(AST::ReturnStatement* ast, TypecheckHelper& tc) {
	typecheck(ast->m_value, tc);

	auto mono = ast->m_value->m_value_type;
	auto seq_expr = ast->m_surrounding_seq_expr;
	tc.unify(seq_expr->m_value_type, mono);
}

static void typecheck_stmt(AST::ExpressionStatement* ast, TypecheckHelper& tc) {
	typecheck(ast->m_expression, tc);
}


static void typecheck_stmt(AST::Declaration* ast, TypecheckHelper& tc) {
	// put a dummy type in the decl to allow recursive definitions
	ast->m_value_type = tc.new_var();
	process_contents(ast, tc);
	generalize(ast, tc);
}

static void typecheck_stmt(AST::Stmt* ast, TypecheckHelper& tc) {
#define DISPATCH(type)                                                         \
	case ASTStmtTag::type:                                                     \
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
	             << ast_string[(int)ast->type()];

#undef DISPATCH
}

void typecheck(AST::AST* ast, TypecheckHelper& tc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                    \
		return typecheck(static_cast<AST::type*>(ast), tc);

#define IGNORE(type)                                                           \
	case ASTTag::type:                                                    \
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
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		IGNORE(BuiltinTypeFunction);
		IGNORE(Constructor);
	}

	Log::fatal() << "(internal) CST type not handled in typecheck: "
	             << ast_string[(int)ast->type()];

#undef DISPATCH
#undef IGNORE
}

void typecheck_program(AST::AST* ast, TypecheckHelper& tc) {
	// NOTE: we don't actually do anything with `ast`: what we really care about
	// has already been precomputed and stored in `tc`. This is not the most
	// friendliest API, so maybe we could look into changing it?

	assert(ast->type() == ASTTag::Program);

	auto const& comps = tc.declaration_order();
	for (auto const& decls : comps) {

		bool type_in_component = false;
		bool non_type_in_component = false;
		for (auto decl : decls) {

			if (tc.is_type(decl->m_meta_type))
				type_in_component = true;
			if (tc.is_term(decl->m_meta_type))
				non_type_in_component = true;
		}

		// we don't deal with types and non-types in the same component.
		if (type_in_component && non_type_in_component)
			Log::fatal() << "found reference cycle with types and values";

		if (type_in_component)
			continue;

		tc.new_nested_scope();

		// set up some dummy types on every decl
		for (auto decl : decls) {
			decl->m_value_type = tc.new_var();
		}

		for (auto decl : decls) {
			process_contents(decl, tc);
		}

		tc.end_scope();

		// generalize all the decl types, so that they are
		// identified as polymorphic in the next rec-block
		for (auto decl : decls) {
			generalize(decl, tc);
		}
	}
}

} // namespace TypeChecker
