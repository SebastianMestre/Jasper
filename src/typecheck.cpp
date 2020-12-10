#include "typecheck.hpp"

#include "compile_time_environment.hpp"
#include "typechecker.hpp"
#include "typechecker_types.hpp"
#include "typed_ast.hpp"

#include <cassert>
#include <iostream>
#include <unordered_set>

namespace TypeChecker {

// NOTE(SMestre): This file duplicates a bit of what
// match_identifiers does. However, I think that's the right
// thing to do. At least, the alternatives I came up with
// are quite a bit worse.

void process_declaration_type_hint(TypedAST::Declaration* ast, TypeChecker& tc);

// Literals

void typecheck(TypedAST::NumberLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_float();
}

void typecheck(TypedAST::IntegerLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_int();
}

void typecheck(TypedAST::StringLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_string();
}

void typecheck(TypedAST::BooleanLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_boolean();
}

void typecheck(TypedAST::NullLiteral* ast, TypeChecker& tc) {
	ast->m_value_type = tc.mono_unit();
}

void typecheck(TypedAST::ArrayLiteral* ast, TypeChecker& tc) {
	auto element_type = tc.new_var();
	for (auto& element : ast->m_elements) {
		typecheck(element, tc);
		tc.m_core.m_mono_core.unify(element_type, element->m_value_type);
	}

	auto array_type =
	    tc.m_core.new_term(BuiltinType::Array, {element_type}, "Array Literal");

	ast->m_value_type = array_type;
}

void typecheck(TypedAST::Identifier* ast, TypeChecker& tc) {
	TypedAST::Declaration* declaration = ast->m_declaration;
	assert(declaration && "accessed an unmatched identifier");

	MetaTypeId meta_type = tc.m_core.m_meta_core.find(declaration->m_meta_type);
	ast->m_meta_type = meta_type;

	assert(meta_type == tc.meta_value());

	// here we implement the [var] rule
	ast->m_value_type = declaration->m_is_polymorphic
	                        ? tc.m_core.inst_fresh(declaration->m_decl_type)
	                        : declaration->m_value_type;
}

void typecheck(TypedAST::Block* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck(child, tc);
	tc.m_env.end_scope();
}

void typecheck(TypedAST::IfElseStatement* ast, TypeChecker& tc) {
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body, tc);

	if (ast->m_else_body)
		typecheck(ast->m_else_body, tc);
}

void typecheck(TypedAST::CallExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_callee, tc);
	for (auto& arg : ast->m_args)
		typecheck(arg, tc);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args)
		arg_types.push_back(arg->m_value_type);

	ast->m_value_type = tc.rule_app(
	    std::move(arg_types), ast->m_callee->m_value_type);
}

void typecheck(TypedAST::FunctionLiteral* ast, TypeChecker& tc) {
	tc.m_env.enter_function(ast);
	tc.m_env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	{
		// TODO: do something better, use the type hints
		ast->m_return_type = tc.new_var();

		int arg_count = ast->m_args.size();
		std::vector<MonoId> arg_types;

		for (int i = 0; i < arg_count; ++i) {
			int mono = tc.new_var();
			arg_types.push_back(mono);
			ast->m_args[i].m_value_type = mono;
		}

		// return type
		arg_types.push_back(ast->m_return_type);

		MonoId term_mono_id = tc.m_core.new_term(
		    BuiltinType::Function, std::move(arg_types));
		ast->m_value_type = term_mono_id;
	}

	// scan body
	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body);
	for (auto& child : body->m_body)
		typecheck(child, tc);

	tc.m_env.end_scope();
	tc.m_env.exit_function();
}

void typecheck(TypedAST::ForStatement* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	typecheck(&ast->m_declaration, tc);
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_action, tc);
	typecheck(ast->m_body, tc);
	tc.m_env.end_scope();
}

void typecheck(TypedAST::WhileStatement* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body, tc);
	tc.m_env.end_scope();
}

void typecheck(TypedAST::ReturnStatement* ast, TypeChecker& tc) {
	typecheck(ast->m_value, tc);

	auto mono = ast->m_value->m_value_type;
	auto func = tc.m_env.current_function();
	tc.m_core.m_mono_core.unify(func->m_return_type, mono);
}

void typecheck(TypedAST::IndexExpression* ast, TypeChecker& tc) {
	// TODO: put the monotype in the ast
	typecheck(ast->m_callee, tc);
	typecheck(ast->m_index, tc);

	auto var = tc.new_var();
	auto arr = tc.m_core.new_term(BuiltinType::Array, {var});
	tc.m_core.m_mono_core.unify(arr, ast->m_callee->m_value_type);

	tc.m_core.m_mono_core.unify(tc.mono_int(), ast->m_index->m_value_type);

	ast->m_value_type = var;
}

void typecheck(TypedAST::TernaryExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_condition, tc);
	tc.m_core.m_mono_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_then_expr, tc);
	typecheck(ast->m_else_expr, tc);

	tc.m_core.m_mono_core.unify(
	    ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

void typecheck(TypedAST::AccessExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_record, tc);

	// should this be a hidden type var?
	MonoId member_type = tc.new_var();
	ast->m_value_type = member_type;

	TypeFunctionId dummy_tf = tc.m_core.new_type_function(
	    TypeFunctionTag::Record,
	    // we don't care about field order in dummies
	    {},
	    {{ast->m_member->m_text, member_type}},
	    true);
	MonoId term_type = tc.m_core.new_term(dummy_tf, {}, "record instance");

	tc.m_core.m_mono_core.unify(ast->m_record->m_value_type, term_type);
}

void typecheck(TypedAST::MatchExpression* ast, TypeChecker& tc) {
	typecheck(&ast->m_matchee, tc);
	if (ast->m_type_hint) {
		assert(ast->m_type_hint->type() == TypedASTTag::MonoTypeHandle);
		auto handle = static_cast<TypedAST::MonoTypeHandle*>(ast->m_type_hint);
		tc.m_core.m_mono_core.unify(ast->m_matchee.m_value_type, handle->m_value);
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
		process_declaration_type_hint(&case_data.m_declaration, tc);

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
	MonoId term_type = tc.m_core.new_term(dummy_tf, {}, "match sum dummy");
	tc.m_core.m_mono_core.unify(ast->m_matchee.m_value_type, term_type);
}

void typecheck(TypedAST::ConstructorExpression* ast, TypeChecker& tc) {
	typecheck(ast->m_constructor, tc);

	auto constructor = static_cast<TypedAST::Constructor*>(ast->m_constructor);
	assert(constructor->type() == TypedASTTag::Constructor);

	TypeFunctionId tf = tc.m_core.m_mono_core.find_function(constructor->m_mono);
	int tf_data_idx = tc.m_core.m_tf_core.find_function(tf);
	TypeFunctionData& tf_data = tc.m_core.m_type_functions[tf_data_idx];

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
		InternedString id = constructor->m_id->m_text;
		MonoId constructor_type = tf_data.structure[id];

		tc.m_core.m_mono_core.unify(constructor_type, ast->m_args[0]->m_value_type);
	}

	ast->m_value_type = constructor->m_mono;
}

void print_information(TypedAST::Declaration* ast, TypeChecker& tc) {
#if DEBUG
	auto poly = ast->m_decl_type;
	auto& poly_data = tc.m_core.poly_data[poly];
	std::cerr << "@@ Type of local variable " << ast->identifier_text() << '\n';
	std::cerr << "@@ Has " << poly_data.vars.size() << " variables\n";
	std::cerr << "@@ It is equal to:\n";
	tc.m_core.m_mono_core.print_node(poly_data.base);
#endif
}

void generalize(TypedAST::Declaration* ast, TypeChecker& tc) {
	assert(!ast->m_is_polymorphic);

	ast->m_is_polymorphic = true;
	ast->m_decl_type = tc.generalize(ast->m_value_type);

	print_information(ast, tc);
}

void process_declaration_type_hint(TypedAST::Declaration* ast, TypeChecker& tc) {
	if (ast->m_type_hint) {
		assert(ast->m_type_hint->type() == TypedASTTag::MonoTypeHandle);
		auto handle = static_cast<TypedAST::MonoTypeHandle*>(ast->m_type_hint);
		tc.m_core.m_mono_core.unify(ast->m_value_type, handle->m_value);
	}
}

// typecheck the value and make the type of the decl equal
// to its type
// apply typehints if available
void process_contents(TypedAST::Declaration* ast, TypeChecker& tc) {
	process_declaration_type_hint(ast, tc);

	// it would be nicer to check this at an earlier stage
	assert(ast->m_value);
	typecheck(ast->m_value, tc);
	tc.m_core.m_mono_core.unify(ast->m_value_type, ast->m_value->m_value_type);
}

void typecheck(TypedAST::Declaration* ast, TypeChecker& tc) {
	// put a dummy type in the decl to allow recursive definitions
	ast->m_value_type = tc.new_var();
	process_contents(ast, tc);
	generalize(ast, tc);
}

void typecheck(TypedAST::DeclarationList* ast, TypeChecker& tc) {

	auto const& comps = tc.m_env.declaration_components;
	for (auto const& decls : comps) {

		bool type_in_component = false;
		bool non_type_in_component = false;
		for (auto decl : decls) {

			auto meta_type = tc.m_core.m_meta_core.find(decl->m_meta_type);
			if (meta_type == tc.meta_typefunc() || meta_type == tc.meta_monotype())
				type_in_component = true;

			if (meta_type == tc.meta_value())
				non_type_in_component = true;
		}

		// we don't deal with types and non-types in the same component.
		assert(!(type_in_component && non_type_in_component));

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
			print_information(decl, tc);
		}
	}
}

void typecheck(TypedAST::TypedAST* ast, TypeChecker& tc) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return typecheck(static_cast<TypedAST::type*>(ast), tc);

#define IGNORE(type)                                                           \
	case TypedASTTag::type:                                                    \
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

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);

		IGNORE(TypeFunctionHandle);
		IGNORE(MonoTypeHandle);
		IGNORE(Constructor);
	}

	std::cerr << "Error: AST type not handled in typecheck: "
	          << typed_ast_string[(int)ast->type()] << std::endl;
	assert(0);

#undef DISPATCH
#undef IGNORE
}

} // namespace TypeChecker
