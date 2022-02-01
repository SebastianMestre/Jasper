#include "typecheck.hpp"

#include "../ast.hpp"
#include "../log/log.hpp"
#include "compile_time_environment.hpp"
#include "typechecker.hpp"
#include "typechecker_types.hpp"

#include <cassert>
#include <unordered_set>

namespace TypeChecker {

static void process_type_hint(AST::Declaration* ast, TypeChecker& tc);

// Literals

void typecheck(AST::NumberLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_float();
}

void typecheck(AST::IntegerLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_int();
}

void typecheck(AST::StringLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_string();
}

void typecheck(AST::BooleanLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_boolean();
}

void typecheck(AST::NullLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_unit();
}

void typecheck(AST::ArrayLiteral* ast, TypeChecker& tc) {
	auto element_type = tc.new_var();
	for (auto& element : ast->m_elements) {
		typecheck(element, tc);
		tc.m_core.m_mono_core.unify(element_type, element->m_value_type);
	}

	auto array_type =
	    tc.m_core.new_term(BuiltinType::Array, {element_type}, "Array Literal");

	ast->m_value_type = array_type;
}

void typecheck(AST::Identifier* ast, TypeChecker& tc) {
	AST::Declaration* declaration = ast->m_declaration;
	assert(declaration);

	auto& uf = tc.m_core.m_meta_core;
	MetaTypeId meta_type = uf.eval(declaration->m_meta_type);
	ast->m_meta_type = meta_type;

	assert(uf.is(meta_type, Tag::Term));

	// here we implement the [var] rule
	ast->m_value_type = declaration->m_is_polymorphic
	                        ? tc.m_core.inst_fresh(declaration->m_decl_type)
	                        : declaration->m_value_type;
}

void typecheck(AST::Block* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck(child, tc);
	tc.m_env.end_scope();
}

void typecheck(AST::IfElseStatement* ast, TypeChecker& tc) {
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body, tc);

	if (ast->m_else_body)
		typecheck(ast->m_else_body, tc);
}

void typecheck(AST::CallExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_callee, tc);
	for (auto& arg : ast->m_args)
		typecheck(arg, tc);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args)
		arg_types.push_back(arg->m_value_type);

	ast->m_value_type = tc.rule_app(
	    std::move(arg_types), ast->m_callee->m_value_type);
}

void typecheck(AST::FunctionLiteral* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	{
		// TODO: consume return-type type-hints
		ast->m_return_type = tc.new_var();

		std::vector<MonoId> arg_types;

		for (auto& arg : ast->m_args) {
			arg.m_value_type = tc.new_var();
			process_type_hint(&arg, tc);
			arg_types.push_back(arg.m_value_type);
		}

		// return type
		arg_types.push_back(ast->m_return_type);

		MonoId term_mono_id = tc.m_core.new_term(
		    BuiltinType::Function, std::move(arg_types));
		ast->m_value_type = term_mono_id;
	}

	// scan body
	typecheck(ast->m_body, tc);
	tc.m_core.m_mono_core.unify(ast->m_return_type, ast->m_body->m_value_type);

	tc.m_env.end_scope();
}

void typecheck(AST::WhileStatement* ast, TypeChecker& tc) {
	// TODO: Why do while statements create a new nested scope?
	tc.m_env.new_nested_scope();
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body, tc);
	tc.m_env.end_scope();
}

void typecheck(AST::ReturnStatement* ast, TypeChecker& tc) {
	typecheck(ast->m_value, tc);

	auto mono = ast->m_value->m_value_type;
	auto seq_expr = ast->m_surrounding_seq_expr;
	tc.m_core.m_mono_core.unify(seq_expr->m_value_type, mono);
}

void typecheck(AST::IndexExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_callee, tc);
	typecheck(ast->m_index, tc);

	auto var = tc.new_var();
	auto arr = tc.m_core.new_term(BuiltinType::Array, {var});
	tc.m_core.m_mono_core.unify(arr, ast->m_callee->m_value_type);

	tc.m_core.m_mono_core.unify(tc.mono_int(), ast->m_index->m_value_type);

	ast->m_value_type = var;
}

void typecheck(AST::TernaryExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_then_expr, tc);
	typecheck(ast->m_else_expr, tc);

	tc.m_core.m_mono_core.unify(
	    ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

void typecheck(AST::AccessExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_target, tc);

	// should this be a hidden type var?
	MonoId member_type = tc.new_var();
	ast->m_value_type = member_type;

	TypeFunctionId dummy_tf = tc.m_core.new_type_function(
	    TypeFunctionTag::Record,
	    // we don't care about field order in dummies
	    {},
	    {{ast->m_member, member_type}},
	    true);
	MonoId term_type = tc.m_core.new_term(dummy_tf, {}, "record instance");

	tc.m_core.m_mono_core.unify(ast->m_target->m_value_type, term_type);
}

void typecheck(AST::MatchExpression* ast, TypeChecker& tc) {
	typecheck(&ast->m_target, tc);
	if (ast->m_type_hint) {
		assert(ast->m_type_hint->type() == ASTTag::MonoTypeHandle);
		auto handle = static_cast<AST::MonoTypeHandle*>(ast->m_type_hint);
		tc.m_core.m_mono_core.unify(ast->m_target.m_value_type, handle->m_value);
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
		tc.m_core.m_mono_core.unify(
		    ast->m_value_type, case_data.m_expression->m_value_type);

		// get the structure of the match expression for a dummy
		dummy_structure[kv.first] = case_data.m_declaration.m_value_type;
	}

	TypeFunctionId dummy_tf = tc.m_core.new_type_function(
	    TypeFunctionTag::Variant,
	    // we don't care about field order in dummies
	    {},
	    std::move(dummy_structure),
	    true);

	// TODO: support user-defined polymorphic datatypes, and the notion of 'not
	// knowing' the arguments to a typefunc.
	MonoId term_type = tc.m_core.new_term(dummy_tf, {}, "match variant dummy");
	tc.m_core.m_mono_core.unify(ast->m_target.m_value_type, term_type);
}

void typecheck(AST::ConstructorExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_constructor, tc);

	auto constructor = static_cast<AST::Constructor*>(ast->m_constructor);
	assert(constructor->type() == ASTTag::Constructor);

	TypeFunctionData& tf_data = tc.m_core.type_function_data_of(constructor->m_mono);

	// match value arguments
	if (tf_data.tag == TypeFunctionTag::Record) {
		assert(tf_data.fields.size() == ast->m_args.size());
		for (int i = 0; i < ast->m_args.size(); ++i) {
			typecheck(ast->m_args[i], tc);
			MonoId field_type = tf_data.structure[tf_data.fields[i]];
			tc.m_core.m_mono_core.unify(field_type, ast->m_args[i]->m_value_type);
		}
	// match the argument type with the constructor used
	} else if (tf_data.tag == TypeFunctionTag::Variant) {
		assert(ast->m_args.size() == 1);

		typecheck(ast->m_args[0], tc);
		InternedString id = constructor->m_id;
		MonoId constructor_type = tf_data.structure[id];

		tc.m_core.m_mono_core.unify(constructor_type, ast->m_args[0]->m_value_type);
	}

	ast->m_value_type = constructor->m_mono;
}

void typecheck(AST::SequenceExpression* ast, TypeChecker& tc) {
	ast->m_value_type = tc.new_var();
	typecheck(ast->m_body, tc);
}

void print_information(AST::Declaration* ast, TypeChecker& tc) {
#if DEBUG
	auto poly = ast->m_decl_type;
	auto& poly_data = tc.m_core.poly_data[poly];
	Log::info() << "Type of local variable '" << ast->identifier_text()
	            << "' has " << poly_data.vars.size() << " type variables";
	Log::info("The type is:");
	tc.m_core.m_mono_core.print_node(poly_data.base);
#endif
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

void generalize(AST::Declaration* ast, TypeChecker& tc) {
	assert(!ast->m_is_polymorphic);

	assert(ast->m_value);

	if (is_value_expression(ast->m_value)) {
		ast->m_is_polymorphic = true;
		ast->m_decl_type = tc.generalize(ast->m_value_type);

		print_information(ast, tc);
	} else {
		// if it's not a value expression, its free vars get bound
		// to the environment instead of being generalized
		tc.bind_free_vars(ast->m_value_type);
	}
}

static void process_type_hint(AST::Declaration* ast, TypeChecker& tc) {
	if (!ast->m_type_hint)
		return;

	assert(ast->m_type_hint->type() == ASTTag::MonoTypeHandle);
	auto handle = static_cast<AST::MonoTypeHandle*>(ast->m_type_hint);
	tc.m_core.m_mono_core.unify(ast->m_value_type, handle->m_value);
}

// typecheck the value and make the type of the decl equal
// to its type
// apply typehints if available
void process_contents(AST::Declaration* ast, TypeChecker& tc) {
	process_type_hint(ast, tc);

	// it would be nicer to check this at an earlier stage
	assert(ast->m_value);
	typecheck(ast->m_value, tc);
	tc.m_core.m_mono_core.unify(ast->m_value_type, ast->m_value->m_value_type);
}

void typecheck(AST::Declaration* ast, TypeChecker& tc) {
	// put a dummy type in the decl to allow recursive definitions
	ast->m_value_type = tc.new_var();
	process_contents(ast, tc);
	generalize(ast, tc);
}

void typecheck(AST::Program* ast, TypeChecker& tc) {

	auto const& comps = tc.m_env.declaration_components;
	for (auto const& decls : comps) {

		bool type_in_component = false;
		bool non_type_in_component = false;
		for (auto decl : decls) {

			auto& uf = tc.m_core.m_meta_core;
			auto meta_type = uf.eval(decl->m_meta_type);
			if (uf.is(meta_type, Tag::Func) || uf.is(meta_type, Tag::Mono))
				type_in_component = true;

			if (uf.is(meta_type, Tag::Term))
				non_type_in_component = true;
		}

		// we don't deal with types and non-types in the same component.
		if (type_in_component && non_type_in_component)
			Log::fatal() << "found reference cycle with types and values";

		if (type_in_component)
			continue;

		// set up some dummy types on every decl
		for (auto decl : decls) {
			decl->m_value_type = tc.new_hidden_var();
		}

		for (auto decl : decls) {
			process_contents(decl, tc);
		}

		// generalize all the decl types, so that they are
		// identified as polymorphic in the next rec-block
		for (auto decl : decls) {
			generalize(decl, tc);
		}
	}
}

void typecheck(AST::AST* ast, TypeChecker& tc) {
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

		DISPATCH(Declaration);
		DISPATCH(Program);

		DISPATCH(Block);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);

		IGNORE(TypeFunctionHandle);
		IGNORE(MonoTypeHandle);
		IGNORE(Constructor);
	}

	Log::fatal() << "(internal) CST type not handled in typecheck: "
	             << ast_string[(int)ast->type()];

#undef DISPATCH
#undef IGNORE
}

} // namespace TypeChecker
