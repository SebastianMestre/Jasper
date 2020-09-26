#include "typecheck.hpp"

#include "compile_time_environment.hpp"
#include "tarjan_solver.hpp"
#include "typechecker.hpp"
#include "typed_ast.hpp"
#include "typechecker_types.hpp"

#include <cassert>
#include <iostream>
#include <unordered_set>

namespace TypeChecker {

// NOTE: This file duplicates a bit of what match_identifiers does.
// however, I think that's the right thing to do.
// At least, the alternative just sucks.

void typecheck(TypedAST::NumberLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
	ast->m_value_type = tc.mono_float();
}

void typecheck(TypedAST::IntegerLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
	ast->m_value_type = tc.mono_int();
}

void typecheck(TypedAST::StringLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
	ast->m_value_type = tc.mono_string();
}

void typecheck(TypedAST::BooleanLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
	ast->m_value_type = tc.mono_boolean();
}

void typecheck(TypedAST::NullLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
	ast->m_value_type = tc.mono_unit();
}

void typecheck(TypedAST::ArrayLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();
	auto element_type = tc.new_var();
	for (auto& element : ast->m_elements) {
		typecheck(element.get(), tc);
		tc.m_core.unify(element_type, element->m_value_type);
	}

	auto array_type =
	    tc.m_core.new_term(BuiltinType::Array, {element_type}, "Array Literal");

	ast->m_value_type = array_type;
}

void typecheck(TypedAST::Declaration* ast, TypeChecker& tc) {
#if DEBUG
	std::cerr << "Typechecking " << ast->identifier_text() << '\n';
#endif

	// FIXME: we should get our meta type from ast->m_value
	ast->m_meta_type = tc.meta_value();

	ast->m_value_type = tc.new_var();

	// this is where we implement rec-polymorphism.
	// TODO: refactor (duplication).
	if (ast->m_value) {
		typecheck(ast->m_value.get(), tc);
		// unify instead of assign. This way, we can do recursion.
		tc.m_core.unify(ast->m_value_type, ast->m_value->m_value_type);
	} else {
		// NOTE: this should be an error at an earlier stage...
	}

	ast->m_is_polymorphic = true;
	ast->m_decl_type = tc.generalize(ast->m_value_type);

#if DEBUG
	{
		auto poly = ast->m_decl_type;
		auto& poly_data = tc.m_core.poly_data[poly];
		std::cerr << "@@ Type of local variable " << ast->identifier_text() << '\n';
		std::cerr << "@@ Has " << poly_data.vars.size() << " variables\n";
		std::cerr << "@@ It is equal to:\n";
		tc.m_core.print_type(poly_data.base);

	}
#endif
}

void typecheck(TypedAST::Identifier* ast, TypeChecker& tc) {
	TypedAST::Declaration* declaration = ast->m_declaration;
	assert(declaration && "accessed an unmatched identifier");

	MetaTypeId meta_type = tc.m_meta_core.find(declaration->m_meta_type);
	ast->m_meta_type = meta_type;
	if (meta_type == tc.meta_value()) {
		// here we implement the [var] rule
		if (declaration->m_is_polymorphic)
			ast->m_value_type = tc.m_core.inst_fresh(declaration->m_decl_type);
		else
			ast->m_value_type = declaration->m_value_type;
	} else if(meta_type == tc.meta_typefunc()){
		// we are a type function.
		// TODO: not too sure what needs to be done...
	} else {
		// meta type var
		// TODO: not too sure what needs to be done...
	}
}

void typecheck(TypedAST::Block* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck(child.get(), tc);
	tc.m_env.end_scope();
}

void typecheck(TypedAST::IfElseStatement* ast, TypeChecker& tc) {
	typecheck(ast->m_condition.get(), tc);
	tc.m_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body.get(), tc);

	if (ast->m_else_body)
		typecheck(ast->m_else_body.get(), tc);
}

void typecheck(TypedAST::CallExpression* ast, TypeChecker& tc) {
	// TODO: do we want to do something special with the meta type?
	ast->m_meta_type = tc.meta_value();

	typecheck(ast->m_callee.get(), tc);
	for (auto& arg : ast->m_args)
		typecheck(arg.get(), tc);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args)
		arg_types.push_back(arg->m_value_type);

	ast->m_value_type = tc.rule_app(
	    std::move(arg_types), ast->m_callee->m_value_type);
}

void typecheck(TypedAST::FunctionLiteral* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	tc.m_env.enter_function(ast);
	tc.m_env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	{
		// TODO: do something better, use the type hints
		ast->m_return_type = tc.new_var();

		int arg_count = ast->m_args.size();
		std::vector<MonoId> arg_types;

		for (int i = 0; i < arg_count; ++i) {
			auto& arg_decl = ast->m_args[i];

			int mono = tc.new_var();

			arg_types.push_back(mono);
			arg_decl.m_value_type = mono;
		}

		// return type
		arg_types.push_back(ast->m_return_type);

		MonoId term_mono_id = tc.m_core.new_term(
		    BuiltinType::Function, std::move(arg_types));
		ast->m_value_type = term_mono_id;
	}

	// scan body
	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body.get());
	for (auto& child : body->m_body)
		typecheck(child.get(), tc);

	tc.m_env.end_scope();
	tc.m_env.exit_function();
}

void typecheck(TypedAST::ForStatement* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	typecheck(ast->m_declaration.get(), tc);
	typecheck(ast->m_condition.get(), tc);
	tc.m_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_action.get(), tc);
	typecheck(ast->m_body.get(), tc);
	tc.m_env.end_scope();
}

void typecheck(TypedAST::WhileStatement* ast, TypeChecker& tc) {
	tc.m_env.new_nested_scope();
	typecheck(ast->m_condition.get(), tc);
	tc.m_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body.get(), tc);
	tc.m_env.end_scope();
}

void typecheck(TypedAST::ReturnStatement* ast, TypeChecker& tc) {
	typecheck(ast->m_value.get(), tc);

	auto mono = ast->m_value->m_value_type;
	auto func = tc.m_env.current_function();
	tc.m_core.unify(func->m_return_type, mono);
}

void typecheck(TypedAST::IndexExpression* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	// TODO: put the monotype in the ast
	typecheck(ast->m_callee.get(), tc);
	typecheck(ast->m_index.get(), tc);
}

void typecheck(TypedAST::TernaryExpression* ast, TypeChecker& tc) {
	ast->m_meta_type = tc.meta_value();

	typecheck(ast->m_condition.get(), tc);
	tc.m_core.unify(
	    ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_then_expr.get(), tc);
	typecheck(ast->m_else_expr.get(), tc);

	tc.m_core.unify(
	    ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

void typecheck(TypedAST::RecordAccessExpression* ast, TypeChecker& tc) {
	// TODO: we want to support static record fields that
	// contain monotypes and type functions, eventually.
	ast->m_meta_type = tc.meta_value();

	typecheck(ast->m_record.get(), tc);

	// should this be a hidden type var?
	MonoId member_type = tc.new_var();
	ast->m_value_type = member_type;

	TypeFunctionId dummy_tf = tc.m_core.new_dummy_type_function(
	    TypeFunctionTag::Record, {{ast->m_member->m_text, member_type}});
	MonoId term_type = tc.m_core.new_term(dummy_tf, {}, "record instance");

	tc.m_core.unify(ast->m_record->m_value_type, term_type);
}

void typecheck(TypedAST::DeclarationList* ast, TypeChecker& tc) {

	// two way mapping
	std::unordered_map<TypedAST::Declaration*, int> decl_to_index;
	std::vector<TypedAST::Declaration*> index_to_decl;

	// assign a unique int to every top level declaration
	int i = 0;
	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		index_to_decl.push_back(d);
		decl_to_index.insert({d, i});
		++i;
	}

	// build up the explicit declaration graph
	TarjanSolver solver(index_to_decl.size());
	for (auto kv : decl_to_index) {
		auto decl = kv.first;
#if DEBUG
		std::cerr << "@@ " << decl->identifier_text() << " -- (" << kv.second
		          << ")\n";
#endif
		auto u = kv.second;
		for (auto other : decl->m_references) {
			auto it = decl_to_index.find(other);
			if (it != decl_to_index.end()) {
				int v = it->second;
#if DEBUG
				std::cerr << "@@   " << other->identifier_text()
				          << " -- add_edge(" << u << ", " << v << ")\n";
#endif
				solver.add_edge(u, v);
			}
		}
	}

	// compute strongly connected components
	solver.solve();

	auto const& comps = solver.vertices_of_components();
	for (auto const& verts : comps) {

#if DEBUG
		std::cerr << "@@@@ TYPECHECKING COMPONENT @@@@\n";
		std::cerr << "@@@@ MEMBER LIST START @@@@\n";
		for (int u : verts) {
			auto decl = index_to_decl[u];
			std::cerr << "  MEMBER: " << decl->identifier_text() << '\n';
		}
		std::cerr << "@@@@ MEMBER LIST END @@@@\n";
#endif

		// set up some dummy types on every decl
		for (int u : verts) {
			auto decl = index_to_decl[u];
			// FIXME: we should get our metatype from decl->m_value
			decl->m_meta_type = tc.meta_value();
			decl->m_value_type = tc.new_hidden_var();
		}

		// typecheck all the values and make the type of the
		// decl equal to the type of their value
		for (int u : verts) {
			auto decl = index_to_decl[u];

			if (decl->m_value) {
				typecheck(decl->m_value.get(), tc);
				tc.m_core.unify(
				    decl->m_value_type, decl->m_value->m_value_type);
			} else {
				// this should be an error...
			}
		}

		// generalize all the decl types, so that they are
		// identified as polymorphic in the next rec-block
		for (int u : verts) {
			auto decl = index_to_decl[u];
			decl->m_is_polymorphic = true;
			decl->m_decl_type =
			    tc.generalize(decl->m_value_type);

#if DEBUG
			{
				auto poly = decl->m_decl_type;
				auto& poly_data = tc.m_core.poly_data[poly];

				std::cerr << "@@ Type of " << decl->identifier_text() << '\n';
				std::cerr << "@@ Has " << poly_data.vars.size() << " variables\n";
				std::cerr << "@@ It is equal to:\n";
				tc.m_core.print_type(poly_data.base);
			}
#endif
		}
	}
}

void typecheck(TypedAST::TypedAST* ast, TypeChecker& tc) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return typecheck(static_cast<TypedAST::type*>(ast), tc);

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
		DISPATCH(RecordAccessExpression);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(IfElseStatement);
		DISPATCH(ReturnStatement);
	}

	std::cerr << "Error: AST type not handled in typecheck: "
	          << typed_ast_string[(int)ast->type()] << std::endl;
	assert(0);

#undef DISPATCH
}

} // namespace TypeChecker
