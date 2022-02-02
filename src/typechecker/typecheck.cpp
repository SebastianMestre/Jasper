#include "typecheck.hpp"

#include "../ast.hpp"
#include "../log/log.hpp"
#include "compile_time_environment.hpp"
#include "typechecker.hpp"
#include "typechecker_types.hpp"

#include <cassert>
#include <unordered_set>

namespace TypeChecker {

struct Facade1 {
	TypeChecker& tc;

	MonoId mono_int();
	MonoId mono_float();
	MonoId mono_string();
	MonoId mono_boolean();
	MonoId mono_unit();

	void bind_free_vars(MonoId mono);
	PolyId generalize(MonoId mono);

	TypeSystemCore& core();
	std::vector<std::vector<AST::Declaration*>> const& declaration_order() const;

	MonoId new_hidden_var();
	MonoId new_var();

	void new_nested_scope();
	void end_scope();
	MonoId rule_app(std::vector<MonoId> args_types, MonoId func_type);
	Frontend::CompileTimeEnvironment& env();

	void unify(MonoId i, MonoId j) { core().m_mono_core.unify(i, j); }

	bool is_type(MetaTypeId i) {
		return meta_type_is(i, Tag::Func) || meta_type_is(i, Tag::Mono);
	}

	bool is_term(MetaTypeId i) {
		return meta_type_is(i, Tag::Term);
	}

	MonoId inst_fresh(PolyId i) {
		return tc.core().inst_fresh(i);
	}

	MonoId new_term(
	    TypeFunctionId type_function,
	    std::vector<MonoId> arguments,
	    char const* debug_data = nullptr) {
		return core().new_term(type_function, std::move(arguments), debug_data);
	}

private:
	bool meta_type_is(MetaTypeId i, Tag t) {
		i = get_resolved_meta_type(i);
		return core().m_meta_core.is(i, t);
	}

	MetaTypeId get_resolved_meta_type(MetaTypeId i) {
		return core().m_meta_core.eval(i);
	}
};

MonoId Facade1::mono_int() { return tc.mono_int(); }
MonoId Facade1::mono_float() { return tc.mono_float(); }
MonoId Facade1::mono_string() { return tc.mono_string(); }
MonoId Facade1::mono_boolean() { return tc.mono_boolean(); }
MonoId Facade1::mono_unit() { return tc.mono_unit(); }

void Facade1::bind_free_vars(MonoId mono) {
	std::unordered_set<MonoId> free_vars;
	core().gather_free_vars(mono, free_vars);
	for (MonoId var : free_vars) {
		if (!env().has_type_var(var)) {
			env().current_scope().m_type_vars.insert(var);
		}
	}
}

// qualifies all free variables in the given monotype
PolyId Facade1::generalize(MonoId mono) {
	std::unordered_set<MonoId> free_vars;
	core().gather_free_vars(mono, free_vars);

	std::vector<MonoId> new_vars;
	std::unordered_map<MonoId, MonoId> mapping;
	for (MonoId var : free_vars) {
		if (!env().has_type_var(var)) {
			auto fresh_var = new_hidden_var();
			new_vars.push_back(fresh_var);
			mapping[var] = fresh_var;
		}
	}

	MonoId base = core().inst_impl(mono, mapping);

	return core().new_poly(base, std::move(new_vars));
}

TypeSystemCore& Facade1::core() { return tc.core(); }
std::vector<std::vector<AST::Declaration*>> const& Facade1::declaration_order() const { return tc.declaration_order(); }

MonoId Facade1::new_hidden_var() { return tc.new_hidden_var(); }
MonoId Facade1::new_var() { return tc.new_var(); }

void Facade1::new_nested_scope() { tc.m_env.new_nested_scope(); }
void Facade1::end_scope() { tc.m_env.end_scope(); }


// Hindley-Milner [App], modified for multiple argument functions.
MonoId Facade1::rule_app(std::vector<MonoId> args_types, MonoId func_type) {
	MonoId return_type = core().m_mono_core.new_var();
	args_types.push_back(return_type);

	MonoId deduced_func_type =
	    core().new_term(BuiltinType::Function, std::move(args_types));

	core().m_mono_core.unify(func_type, deduced_func_type);

	return return_type;
}
Frontend::CompileTimeEnvironment& Facade1::env() { return tc.env(); }


void typecheck(AST::AST* ast, Facade1& tc);

void typecheck(AST::AST* ast, TypeChecker& tc) {
	Facade1 f = {tc};
	typecheck(ast, f);
}

static void process_type_hint(AST::Declaration* ast, Facade1& tc);


// Literals
void typecheck(AST::NumberLiteral* ast, Facade1& tc) {
	ast->m_value_type = tc.mono_float();
}

void typecheck(AST::IntegerLiteral* ast, Facade1& tc) {
	ast->m_value_type = tc.mono_int();
}

void typecheck(AST::StringLiteral* ast, Facade1& tc) {
	ast->m_value_type = tc.mono_string();
}

void typecheck(AST::BooleanLiteral* ast, Facade1& tc) {
	ast->m_value_type = tc.mono_boolean();
}

void typecheck(AST::NullLiteral* ast, Facade1& tc) {
	ast->m_value_type = tc.mono_unit();
}

void typecheck(AST::ArrayLiteral* ast, Facade1& tc) {
	auto element_type = tc.new_var();
	for (auto& element : ast->m_elements) {
		typecheck(element, tc);
		tc.unify(element_type, element->m_value_type);
	}

	ast->m_value_type =
	    tc.new_term(BuiltinType::Array, {element_type}, "Array Literal");
}

void typecheck(AST::Identifier* ast, Facade1& tc) {
	AST::Declaration* declaration = ast->m_declaration;
	assert(declaration);

	assert(tc.is_term(declaration->m_meta_type));

	// here we implement the [var] rule
	ast->m_value_type = declaration->m_is_polymorphic
	                        ? tc.inst_fresh(declaration->m_decl_type)
	                        : declaration->m_value_type;
}

void typecheck(AST::Block* ast, Facade1& tc) {
	tc.new_nested_scope();
	for (auto& child : ast->m_body)
		typecheck(child, tc);
	tc.end_scope();
}

void typecheck(AST::IfElseStatement* ast, Facade1& tc) {
	typecheck(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body, tc);

	if (ast->m_else_body)
		typecheck(ast->m_else_body, tc);
}

void typecheck(AST::CallExpression* ast, Facade1& tc) {
	typecheck(ast->m_callee, tc);
	for (auto& arg : ast->m_args)
		typecheck(arg, tc);

	std::vector<MonoId> arg_types;
	for (auto& arg : ast->m_args)
		arg_types.push_back(arg->m_value_type);

	ast->m_value_type = tc.rule_app(
	    std::move(arg_types), ast->m_callee->m_value_type);
}

void typecheck(AST::FunctionLiteral* ast, Facade1& tc) {
	tc.new_nested_scope(); // NOTE: this is nested because of lexical scoping

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

		ast->m_value_type =
		    tc.new_term(BuiltinType::Function, std::move(arg_types));
	}

	// scan body
	typecheck(ast->m_body, tc);
	tc.unify(ast->m_return_type, ast->m_body->m_value_type);

	tc.end_scope();
}

void typecheck(AST::WhileStatement* ast, Facade1& tc) {
	// TODO: Why do while statements create a new nested scope?
	tc.new_nested_scope();
	typecheck(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_body, tc);
	tc.end_scope();
}

void typecheck(AST::ReturnStatement* ast, Facade1& tc) {
	typecheck(ast->m_value, tc);

	auto mono = ast->m_value->m_value_type;
	auto seq_expr = ast->m_surrounding_seq_expr;
	tc.unify(seq_expr->m_value_type, mono);
}

void typecheck(AST::IndexExpression* ast, Facade1& tc) {
	typecheck(ast->m_callee, tc);
	typecheck(ast->m_index, tc);

	auto var = tc.new_var();
	auto arr = tc.new_term(BuiltinType::Array, {var});
	tc.unify(arr, ast->m_callee->m_value_type);

	tc.unify(tc.mono_int(), ast->m_index->m_value_type);

	ast->m_value_type = var;
}

void typecheck(AST::TernaryExpression* ast, Facade1& tc) {
	typecheck(ast->m_condition, tc);
	tc.unify(ast->m_condition->m_value_type, tc.mono_boolean());

	typecheck(ast->m_then_expr, tc);
	typecheck(ast->m_else_expr, tc);

	tc.unify(ast->m_then_expr->m_value_type, ast->m_else_expr->m_value_type);

	ast->m_value_type = ast->m_then_expr->m_value_type;
}

void typecheck(AST::AccessExpression* ast, Facade1& tc) {
	typecheck(ast->m_target, tc);

	// should this be a hidden type var?
	MonoId member_type = tc.new_var();
	ast->m_value_type = member_type;

	TypeFunctionId dummy_tf = tc.core().new_type_function(
	    TypeFunctionTag::Record,
	    // we don't care about field order in dummies
	    {},
	    {{ast->m_member, member_type}},
	    true);
	MonoId term_type = tc.new_term(dummy_tf, {}, "record instance");

	tc.unify(ast->m_target->m_value_type, term_type);
}

void typecheck(AST::MatchExpression* ast, Facade1& tc) {
	typecheck(&ast->m_target, tc);
	if (ast->m_type_hint) {
		assert(ast->m_type_hint->type() == ASTTag::MonoTypeHandle);
		auto handle = static_cast<AST::MonoTypeHandle*>(ast->m_type_hint);
		tc.unify(ast->m_target.m_value_type, handle->m_value);
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

	TypeFunctionId dummy_tf = tc.core().new_type_function(
	    TypeFunctionTag::Variant,
	    // we don't care about field order in dummies
	    {},
	    std::move(dummy_structure),
	    true);

	// TODO: support user-defined polymorphic datatypes, and the notion of 'not
	// knowing' the arguments to a typefunc.
	MonoId term_type = tc.new_term(dummy_tf, {}, "match variant dummy");
	tc.unify(ast->m_target.m_value_type, term_type);
}

void typecheck(AST::ConstructorExpression* ast, Facade1& tc) {
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

void typecheck(AST::SequenceExpression* ast, Facade1& tc) {
	ast->m_value_type = tc.new_var();
	typecheck(ast->m_body, tc);
}

void print_information(AST::Declaration* ast, Facade1& tc) {
#if DEBUG
	auto poly = ast->m_decl_type;
	auto& poly_data = tc.core().poly_data[poly];
	Log::info() << "Type of local variable '" << ast->identifier_text()
	            << "' has " << poly_data.vars.size() << " type variables";
	Log::info("The type is:");
	tc.core().m_mono_core.print_node(poly_data.base);
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

void generalize(AST::Declaration* ast, Facade1& tc) {
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

static void process_type_hint(AST::Declaration* ast, Facade1& tc) {
	if (!ast->m_type_hint)
		return;

	assert(ast->m_type_hint->type() == ASTTag::MonoTypeHandle);
	auto handle = static_cast<AST::MonoTypeHandle*>(ast->m_type_hint);
	tc.unify(ast->m_value_type, handle->m_value);
}

// typecheck the value and make the type of the decl equal
// to its type
// apply typehints if available
void process_contents(AST::Declaration* ast, Facade1& tc) {
	process_type_hint(ast, tc);

	// it would be nicer to check this at an earlier stage
	assert(ast->m_value);
	typecheck(ast->m_value, tc);
	tc.unify(ast->m_value_type, ast->m_value->m_value_type);
}

void typecheck(AST::Declaration* ast, Facade1& tc) {
	// put a dummy type in the decl to allow recursive definitions
	ast->m_value_type = tc.new_var();
	process_contents(ast, tc);
	generalize(ast, tc);
}

void typecheck(AST::Program* ast, Facade1& tc) {

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

void typecheck(AST::AST* ast, Facade1& tc) {
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
