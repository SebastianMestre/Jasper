#include "ct_eval.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../log/log.hpp"
#include "typechecker.hpp"

#include <iostream>
#include <sstream>

#include <cassert>

namespace TypeChecker {

using AST::ExprTag;
using AST::StmtTag;

static void ct_visit(AST::Stmt*& ast, TypeChecker& tc, AST::Allocator& alloc);

static void do_the_thing(AST::Program* ast, TypeChecker& tc, AST::Allocator& alloc);

void reify_types(AST::Program* ast, TypeChecker& tc, AST::Allocator& alloc) {
	do_the_thing(ast, tc, alloc);
}

static AST::Expr* ct_eval(AST::Expr* ast, TypeChecker& tc, AST::Allocator& alloc);
static void ct_visit(AST::Block* ast, TypeChecker& tc, AST::Allocator& alloc);

static AST::Constructor* constructor_from_ast(AST::Expr* ast, TypeChecker& tc, AST::Allocator& alloc);

static MonoId compute_mono(AST::Expr*, TypeChecker&);
static TypeFunctionId compute_type_func(AST::Expr*, TypeChecker&);


// literals

static AST::ArrayLiteral* ct_eval(
    AST::ArrayLiteral* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& element : ast->m_elements)
		element = ct_eval(element, tc, alloc);
	return ast;
}

// expressions

static AST::FunctionLiteral* ct_eval(
    AST::FunctionLiteral* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& arg : ast->m_args)
		if (arg.m_type_hint)
			arg.m_type_hint = ct_eval(arg.m_type_hint, tc, alloc);

	ast->m_body = ct_eval(ast->m_body, tc, alloc);

	return ast;
}

static AST::Expr* ct_eval(
    AST::Identifier* ast, TypeChecker& tc, AST::Allocator& alloc) {
	// This does something very similar to what ct_eval(Program) does.
	// Maybe it can be de-duplicated?

	assert(ast);
	assert(ast->m_declaration);

	MetaType meta_type = ast->m_declaration->m_meta_type;

	assert(meta_type != MetaType::Undefined);

	if (meta_type == MetaType::Term) {
		return ast;
	} else if (meta_type == MetaType::Type) {
		return ast->m_declaration->m_value;
	} else if (meta_type == MetaType::TypeFunction) {
		return ast->m_declaration->m_value;
	}

	assert(0 && "UNREACHABLE");
}

static AST::CallExpression* ct_eval(
    AST::CallExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	ast->m_callee = ct_eval(ast->m_callee, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = ct_eval(arg, tc, alloc);

	return ast;
}

static AST::IndexExpression* ct_eval(
    AST::IndexExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_callee = ct_eval(ast->m_callee, tc, alloc);
	ast->m_index = ct_eval(ast->m_index, tc, alloc);

	return ast;
}

static AST::TernaryExpression* ct_eval(
    AST::TernaryExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_then_expr = ct_eval(ast->m_then_expr, tc, alloc);
	ast->m_else_expr = ct_eval(ast->m_else_expr, tc, alloc);
	return ast;
}

static AST::Expr* ct_eval(
    AST::AccessExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	MetaType meta_type = ast->m_meta_type;

	if (meta_type == MetaType::Constructor)
		return constructor_from_ast(ast, tc, alloc);

	ast->m_target = ct_eval(ast->m_target, tc, alloc);
	return ast;
}

static AST::Expr* ct_eval(
    AST::MatchExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	// NOTE: no need to ct_eval the identifier, because it is guaranteed
	// to be of metatype value, thus nothing needs to be done

	if (ast->m_type_hint) {
		ast->m_type_hint = ct_eval(ast->m_type_hint, tc, alloc);
	}

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		// TODO(SMestre): factor this out (it's repeated in a bunch of other places)
		if (case_data.m_declaration.m_type_hint) {
			case_data.m_declaration.m_type_hint =
			    ct_eval(case_data.m_declaration.m_type_hint, tc, alloc);
		}

		case_data.m_expression = ct_eval(case_data.m_expression, tc, alloc);
	}

	return ast;
}

static AST::ConstructorExpression* ct_eval(
    AST::ConstructorExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_constructor = constructor_from_ast(ast->m_constructor, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = ct_eval(arg, tc, alloc);

	return ast;
}

static AST::Expr* ct_eval(
    AST::SequenceExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ct_visit(ast->m_body, tc, alloc);
	return ast;
}
// types

static AST::StructExpression* ct_eval(
    AST::StructExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_value = compute_type_func(ast, tc);
	return ast;
}

static AST::UnionExpression* ct_eval(
    AST::UnionExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_value = compute_type_func(ast, tc);
	return ast;
}

static AST::Constructor* constructor_from_ast(
    AST::Expr* ast, TypeChecker& tc, AST::Allocator& alloc) {
	MetaType meta = ast->m_meta_type;
	auto constructor = alloc.make<AST::Constructor>();
	constructor->m_syntax = ast;

	if (meta == MetaType::Type) {
		constructor->m_mono = compute_mono(ast, tc);
	} else if (meta == MetaType::Constructor) {
		assert(ast->type() == ExprTag::AccessExpression);

		auto access = static_cast<AST::AccessExpression*>(ast);

		auto case_name = access->m_member;

		MonoId expected_ty = tc.core().ll_new_var();
		VarId v = tc.core().get_var_id(expected_ty);

		tc.core().add_variant_constraint(v);
		tc.core().add_field_constraint(v, case_name, tc.core().ll_new_var());

		MonoId actual_ty = compute_mono(access->m_target, tc);

		tc.core().ll_unify(expected_ty, actual_ty);

		constructor->m_mono = actual_ty;
		constructor->m_id = case_name;
	} else {
		Log::fatal() << "Constructor invokation on a non-constructor -- MetaType(" << int(meta) << ")";
	}

	return constructor;
}

static AST::TypeTerm* ct_eval(AST::TypeTerm* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_value = compute_mono(ast, tc);
	return ast;
}

// statements

static void ct_visit(AST::Block* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& child : ast->m_body) {
		ct_visit(child, tc, alloc);
	}
}

static void ct_visit(AST::IfElseStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ct_visit(ast->m_body, tc, alloc);
	if (ast->m_else_body) {
		ct_visit(ast->m_else_body, tc, alloc);
	}
}

static void ct_visit(AST::WhileStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ct_visit(ast->m_body, tc, alloc);
}

static void ct_visit(AST::ReturnStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_value = ct_eval(ast->m_value, tc, alloc);
}

static void ct_visit(AST::ExpressionStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_expression = ct_eval(ast->m_expression, tc, alloc);
}

static void ct_visit(AST::Declaration* ast, TypeChecker& tc, AST::Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = ct_eval(ast->m_type_hint, tc, alloc);

	ast->m_value = ct_eval(ast->m_value, tc, alloc);
}

static TypeFunctionId get_type_function_id(AST::Expr* ast) {
	switch (ast->type()) {

	case ExprTag::StructExpression: {
		auto struct_expression = static_cast<AST::StructExpression*>(ast);
		return struct_expression->m_value;
	}

	case ExprTag::UnionExpression: {
		auto union_expression = static_cast<AST::UnionExpression*>(ast);
		return union_expression->m_value;
	}

	case ExprTag::Identifier: {
		auto identifier = static_cast<AST::Identifier*>(ast);
		return get_type_function_id(identifier->m_declaration->m_value);
	}
	
	case ExprTag::BuiltinTypeFunction: {
		auto handle = static_cast<AST::BuiltinTypeFunction*>(ast);
		return handle->m_value;
	}
	
	default:
		assert(0);
		break;
	}
}

static void stub_type_function_id(AST::Expr* ast, TypeChecker& tc) {
	switch (ast->type()) {

	case ExprTag::StructExpression: {
		auto struct_expression = static_cast<AST::StructExpression*>(ast);
		struct_expression->m_value = tc.core().new_type_function_var();
		break;
	}

	case ExprTag::UnionExpression: {
		auto union_expression = static_cast<AST::UnionExpression*>(ast);
		union_expression->m_value = tc.core().new_type_function_var();
		break;
	}

	case ExprTag::Identifier: {
		auto identifier = static_cast<AST::Identifier*>(ast);
		stub_type_function_id(identifier->m_declaration->m_value, tc);
		break;
	}
	
	default:
		assert(0);
		break;
	}
}

static void stub_monotype_id(AST::Expr* ast, TypeChecker& tc) {
	switch (ast->type()) {
	case ExprTag::TypeTerm:
		return void(static_cast<AST::TypeTerm*>(ast)->m_value = tc.new_var());
	case ExprTag::Identifier:
		return void(stub_monotype_id(static_cast<AST::Identifier*>(ast)->m_declaration->m_value, tc));
	default: assert(0);
	}
}

static MonoId get_monotype_id(AST::Expr* ast) {
	switch (ast->type()) {
	case ExprTag::TypeTerm:
		return static_cast<AST::TypeTerm*>(ast)->m_value;
	case ExprTag::Identifier:
		return get_monotype_id(static_cast<AST::Identifier*>(ast)->m_declaration->m_value);
	default: assert(0);
	}
}

static void do_the_thing(AST::Program* ast, TypeChecker& tc, AST::Allocator& alloc) {

	for (auto& decl : ast->m_declarations) {
		MetaType meta_type = decl.m_meta_type;

#if 0
		std::cerr << "[ Pass 1 ] top level \"" << decl.m_identifier << "\"\n";
		std::cerr << "           metatype tag is: Tag(" << int(meta_type) << ")\n";
#endif

		assert(meta_type != MetaType::Undefined);

		// put a dummy var where required.
		if (meta_type == MetaType::TypeFunction) {
			stub_type_function_id(decl.m_value, tc);
		} else if (meta_type == MetaType::Type) {
			stub_monotype_id(decl.m_value, tc);
		}
	}

	auto const& comps = tc.declaration_order();
	for (auto const& decls : comps) {
		for (auto decl : decls) {
			MetaType meta_type = decl->m_meta_type;

#if 0
			std::cerr << "[ Pass 2 ] top level \"" << decl->m_identifier << "\"\n";
			std::cerr << "           metatype tag is: Tag(" << int(meta_type) << ")\n";
#endif

			assert(meta_type != MetaType::Undefined);

			if (meta_type == MetaType::TypeFunction) {
				if (decl->m_type_hint)
					Log::fatal() << "type hint not allowed in typefunc declaration";

				TypeFunctionId tf = compute_type_func(decl->m_value, tc);
				tc.core().unify_type_function(tf, get_type_function_id(decl->m_value));
			} else if (meta_type == MetaType::Type) {
				if (decl->m_type_hint)
					Log::fatal() << "type hint not allowed in type declaration";

				MonoId mt = compute_mono(decl->m_value, tc);
				tc.core().ll_unify(mt, get_monotype_id(decl->m_value));
			} else {
				if (decl->m_type_hint)
					decl->m_type_hint = ct_eval(decl->m_type_hint, tc, alloc);
				decl->m_value = ct_eval(decl->m_value, tc, alloc);
			}
		}
	}
}

static void ct_visit(AST::Stmt*& ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case StmtTag::type:                                                        \
		return ct_visit(static_cast<AST::type*>(ast), tc, alloc)

	switch (ast->tag()) {
		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);
		DISPATCH(ExpressionStatement);

		DISPATCH(Declaration);

	}
	assert(false);

#undef DISPATCH
}

static AST::Expr* ct_eval(AST::Expr* ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ExprTag::type:                                                        \
		return ct_eval(static_cast<AST::type*>(ast), tc, alloc)

#define RETURN(type)                                                           \
	case ExprTag::type:                                                        \
		return static_cast<AST::type*>(ast);

#define REJECT(type)                                                           \
	case ExprTag::type:                                                        \
		assert(0);

	switch (ast->type()) {
		RETURN(IntegerLiteral);
		RETURN(NumberLiteral);
		RETURN(BooleanLiteral);
		RETURN(StringLiteral);
		RETURN(NullLiteral);
		DISPATCH(ArrayLiteral);

		DISPATCH(Identifier);
		DISPATCH(FunctionLiteral);
		DISPATCH(CallExpression);
		DISPATCH(IndexExpression);
		DISPATCH(TernaryExpression);
		DISPATCH(AccessExpression);
		DISPATCH(MatchExpression);
		DISPATCH(ConstructorExpression);
		DISPATCH(SequenceExpression);

		DISPATCH(TypeTerm);
		DISPATCH(StructExpression);
		DISPATCH(UnionExpression);
		REJECT(BuiltinTypeFunction);
		REJECT(Constructor);
	}

	Log::fatal() << "(internal) Unhandled case in ct_eval : "
	             << AST::expr_string[int(ast->type())];

#undef DISPATCH
#undef RETURN
}

static std::unordered_map<InternedString, MonoId> build_map(
    std::vector<InternedString> const&, std::vector<AST::Expr*> const&, TypeChecker&);

static TypeFunctionId compute_type_func(AST::Identifier* ast, TypeChecker& tc) {
	assert(ast->m_declaration);
	assert(ast->m_meta_type == MetaType::TypeFunction);
	auto decl = ast->m_declaration;
	return get_type_function_id(decl->m_value);
}

static TypeFunctionId compute_type_func(AST::StructExpression* ast, TypeChecker& tc) {
	return tc.core().new_type_function_for_ct_eval1(
	    ast->m_fields, build_map(ast->m_fields, ast->m_types, tc));
}

static TypeFunctionId compute_type_func(AST::UnionExpression* ast, TypeChecker& tc) {
	return tc.core().new_type_function_for_ct_eval2(
	    build_map(ast->m_constructors, ast->m_types, tc));
}

static TypeFunctionId compute_type_func(AST::Expr* ast, TypeChecker& tc) {
	assert(
	    ast->type() == ExprTag::Identifier ||
	    ast->type() == ExprTag::UnionExpression ||
	    ast->type() == ExprTag::StructExpression);

	if (ast->type() == ExprTag::UnionExpression) {
		return compute_type_func(static_cast<AST::UnionExpression*>(ast), tc);
	} else if (ast->type() == ExprTag::StructExpression) {
		return compute_type_func(static_cast<AST::StructExpression*>(ast), tc);
	} else {
		return compute_type_func(static_cast<AST::Identifier*>(ast), tc);
	}
}

static MonoId compute_mono(AST::Identifier* ast, TypeChecker& tc) {
	assert(ast->m_declaration);
	assert(ast->m_meta_type == MetaType::Type);
	return get_monotype_id(ast->m_declaration->m_value);
}

static MonoId compute_mono(AST::TypeTerm* ast, TypeChecker& tc) {
	TypeFunctionId type_function = compute_type_func(ast->m_callee, tc);

	std::vector<MonoId> args;
	for (auto& arg : ast->m_args) {
		args.push_back(compute_mono(arg, tc));
	}

	return tc.core().new_term(type_function, std::move(args), "from ast");
}

static MonoId compute_mono(AST::Expr* ast, TypeChecker& tc) {
	assert(ast);
	assert(ast->type() == ExprTag::Identifier || ast->type() == ExprTag::TypeTerm);
	if (ast->type() == ExprTag::Identifier) {
		return compute_mono(static_cast<AST::Identifier*>(ast), tc);
	} else {
		return compute_mono(static_cast<AST::TypeTerm*>(ast), tc);
	}
}

static std::unordered_map<InternedString, MonoId> build_map(
    std::vector<InternedString> const& names,
    std::vector<AST::Expr*> const& types,
    TypeChecker& tc) {

	assert(names.size() == types.size());

	std::unordered_map<InternedString, MonoId> structure;
	int n = names.size();
	for (int i = 0; i < n; ++i){
		MonoId mono = compute_mono(types[i], tc);
		InternedString name = names[i];
		assert(!structure.count(name));
		structure[name] = mono;
	}

	return structure;
}

} // namespace TypeChecker
