#include "typecheck.hpp"

#include "compile_time_environment.hpp"
#include "tarjan_solver.hpp"
#include "typed_ast.hpp"
#include "typesystem_types.hpp"

#include <unordered_set>
#include <cassert>

#if DEBUG
#include <iostream>
#endif

namespace TypeChecker {

// NOTE: This file duplicates a bit of what match_identifiers does.
// however, I think that's the right thing to do.
// At least, the alternative just sucks.

void typecheck(TypedAST::NumberLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_float();
}

void typecheck(TypedAST::IntegerLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_int();
}

void typecheck(TypedAST::StringLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_string();
}

void typecheck(TypedAST::BooleanLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_boolean();
}

void typecheck(TypedAST::NullLiteral* ast, Frontend::CompileTimeEnvironment& env) {
	ast->m_value_type = env.m_typechecker.mono_unit();
}

void typecheck(TypedAST::Declaration* ast, Frontend::CompileTimeEnvironment& env) {
#if DEBUG
	std::cerr << "Typechecking " << ast->identifier_text() << '\n';
#endif

	// we use a hidden typevar to get it to generalize if needed
	ast->m_value_type = env.new_hidden_type_var();
	env.declare(ast->identifier_text(), ast);

	// this is where we implement rec-polymorphism.
	// TODO: refactor (duplication).
	if (ast->m_value) {
		typecheck(ast->m_value.get(), env);
		ast->m_value_type = ast->m_value->m_value_type;
	} else {
		// NOTE: this should be an error...
	}

	ast->m_is_polymorphic = true;
	ast->m_decl_type = env.m_typechecker.m_core.generalize(ast->m_value_type, env);

#if DEBUG
	{
		std::cerr << "Type of " << ast->identifier_text() << " is:\n";
		auto poly = ast->m_decl_type;
		auto mono = env.m_typechecker.m_core.poly_data[poly].base;
		env.m_typechecker.m_core.print_type(mono);
	}
#endif
}

void typecheck(TypedAST::Identifier* ast, Frontend::CompileTimeEnvironment& env) {
	Frontend::Binding* binding = env.access_binding(ast->text());
	assert(binding && "accessed an undeclared identifier");

	// here we implement the [var] rule
	// TODO: refactor
	MonoId mono = -1;
	if (binding->m_type == Frontend::BindingType::Declaration) {
		TypedAST::Declaration* decl = binding->get_decl();
		assert(decl);
		if (decl->m_is_polymorphic) {
			mono = env.m_typechecker.m_core.inst_fresh(decl->m_decl_type);
		} else {
			mono = decl->m_value_type;
		}
	} else {
		TypedAST::FunctionArgument& arg = binding->get_arg();
		mono = arg.m_value_type;
	}

	assert(mono != -1);
	ast->m_value_type = mono;
}

void typecheck(TypedAST::Block* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck(child.get(), env);
	env.end_scope();
}

void typecheck(TypedAST::IfStatement* ast, Frontend::CompileTimeEnvironment& env) {
	typecheck(ast->m_condition.get(), env);
	env.m_typechecker.m_core.unify(
	    ast->m_condition->m_value_type, env.m_typechecker.mono_boolean());

	typecheck(ast->m_body.get(), env);
}

void typecheck(TypedAST::CallExpression* ast, Frontend::CompileTimeEnvironment& env) {
	typecheck(ast->m_callee.get(), env);
	for (auto& arg : ast->m_args)
		typecheck(arg.get(), env);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args)
		arg_types.push_back(arg->m_value_type);

	ast->m_value_type = env.m_typechecker.rule_app(
	    std::move(arg_types), ast->m_callee->m_value_type);
}

void typecheck(TypedAST::FunctionLiteral* ast, Frontend::CompileTimeEnvironment& env) {

	env.enter_function(ast);
	env.new_nested_scope(); // NOTE: this is nested because of lexical scoping

	{
		// TODO: do something better, use the type hints
		ast->m_return_type = env.new_type_var();

		int arg_count = ast->m_args.size();
		std::vector<MonoId> arg_types;

		for (int i = 0; i < arg_count; ++i) {
			auto& arg_decl = ast->m_args[i];

			int mono = env.new_type_var();

			arg_types.push_back(mono);
			arg_decl.m_value_type = mono;

			env.declare_arg(arg_decl.identifier_text(), ast, i);
		}

		// return type
		arg_types.push_back(ast->m_return_type);

		MonoId term_mono_id = env.m_typechecker.m_core.new_term(
		    BuiltinType::Function, std::move(arg_types));
		ast->m_value_type = term_mono_id;
	}

	// scan body
	assert(ast->m_body->type() == ast_type::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body.get());
	for (auto& child : body->m_body)
		typecheck(child.get(), env);

	env.end_scope();
	env.exit_function();
}

void typecheck(TypedAST::ForStatement* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	typecheck(ast->m_declaration.get(), env);
	typecheck(ast->m_condition.get(), env);
	env.m_typechecker.m_core.unify(
	    ast->m_condition->m_value_type, env.m_typechecker.mono_boolean());

	typecheck(ast->m_action.get(), env);
	typecheck(ast->m_body.get(), env);
	env.end_scope();
}

void typecheck(TypedAST::WhileStatement* ast, Frontend::CompileTimeEnvironment& env) {
	env.new_nested_scope();
	typecheck(ast->m_condition.get(), env);
	env.m_typechecker.m_core.unify(
	    ast->m_condition->m_value_type, env.m_typechecker.mono_boolean());

	typecheck(ast->m_body.get(), env);
	env.end_scope();
}

void typecheck(TypedAST::ReturnStatement* ast, Frontend::CompileTimeEnvironment& env) {
	typecheck(ast->m_value.get(), env);

	auto mono = ast->m_value->m_value_type;
	auto func = env.current_function();
	env.m_typechecker.m_core.unify(func->m_return_type, mono);
}

void typecheck(TypedAST::IndexExpression* ast, Frontend::CompileTimeEnvironment& env) {
	typecheck(ast->m_callee.get(), env);
	typecheck(ast->m_index.get(), env);
}

#define USE_REC_RULE 1

void typecheck(TypedAST::DeclarationList* ast, Frontend::CompileTimeEnvironment& env) {

#if USE_REC_RULE
	// two way mapping
	std::unordered_map<TypedAST::Declaration*, int> decl_to_index;
	std::vector<TypedAST::Declaration*> index_to_decl;

	// assign a unique int to every top level declaration
	int i = 0;
	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		index_to_decl.push_back(d);
		decl_to_index.insert({ d, i });
		++i;
	}

	// build up the explicit declaration graph
	TarjanSolver solver(index_to_decl.size());
	for (auto kv : decl_to_index) {
		auto decl = kv.first;
		auto u = kv.second;
		for (auto other : decl->m_references) {
			auto it = decl_to_index.find(other);
			if (it != decl_to_index.end()) {
				int v = it->second;
				solver.add_edge(u, v);
			}
		}
	}

	// compute strongly connected components
	solver.solve();

	auto const& comps = solver.vertices_of_components();
	for (auto const& verts : comps) {

		// set up some dummy types on every decl
		for (int u : verts) {
			auto decl = index_to_decl[u];
			decl->m_value_type = env.new_hidden_type_var();
		}

		// typecheck all the values and make the type of the
		// decl equal to the type of their value
		for (int u : verts) {
			auto decl = index_to_decl[u];

			if (decl->m_value) {
				typecheck(decl->m_value.get(), env);
				env.m_typechecker.m_core.unify(
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
			decl->m_decl_type
			    = env.m_typechecker.m_core.generalize(decl->m_value_type, env);
		}

	}
#else

	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		env.declare(d->identifier_text(), d);
	}

	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedAST::Declaration*>(decl.get());
		d->m_is_polymorphic = true;

		// this is where we implement let-polymorphism. TODO: refactor (duplication).
		if (d->m_value)
			typecheck(d->m_value.get(), env);

		MonoId mono = d->m_value ? d->m_value->m_value_type
		                         : env.new_type_var();

		d->m_decl_type = env.m_typechecker.m_core.generalize(mono, env);

#    if DEBUG
		{
			auto poly = d->m_decl_type;
			auto& poly_data = env.m_typechecker.m_core.poly_data[poly];

			std::cerr << "@@ Type of " << d->identifier_text() << '\n';
			std::cerr << "@@ Has " << poly_data.vars.size() << " variables\n";
			std::cerr << "@@ It is equal to:\n";
			env.m_typechecker.m_core.print_type(mono);
		}
#    endif
	}
#endif
}

void typecheck(TypedAST::TypedAST* ast, Frontend::CompileTimeEnvironment& env) {
#define DISPATCH(type)                                                         \
	case ast_type::type:                                                       \
		return typecheck(static_cast<TypedAST::type*>(ast), env);

	// TODO: Compound literals
	switch (ast->type()) {
		DISPATCH(NumberLiteral);
		DISPATCH(IntegerLiteral);
		DISPATCH(StringLiteral);
		DISPATCH(BooleanLiteral);
		DISPATCH(NullLiteral);

		DISPATCH(Declaration);
		DISPATCH(Identifier);
		DISPATCH(Block);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(IfStatement);
		DISPATCH(FunctionLiteral);
		DISPATCH(CallExpression);
		DISPATCH(ReturnStatement);
		DISPATCH(IndexExpression);
		DISPATCH(DeclarationList);
	case ast_type::BinaryExpression:
		assert(0);
		return;
	}

#undef DISPATCH
}

} // namespace TypeChecker
