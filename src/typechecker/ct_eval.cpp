#include "ct_eval.hpp"

#include "../ast.hpp"
#include "../ast_allocator.hpp"
#include "../log/log.hpp"
#include "typechecker.hpp"

#include <iostream>
#include <sstream>

#include <cassert>

namespace TypeChecker {

static void ct_visit(AST::AST*& ast, TypeChecker& tc, AST::Allocator& alloc);

void reify_types(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ct_visit(ast, tc, alloc);
}

static AST::Expr* ct_eval(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc);
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

	auto& uf = tc.core().m_meta_core;
	MetaTypeId meta_type = uf.eval(ast->m_meta_type);

	if (!uf.is_constant(meta_type))
		Log::fatal() << "Incomplete type inference on identifier" << ast->text();

	if (uf.is(meta_type, Tag::Term)) {
		return ast;
	} else if (uf.is(meta_type, Tag::Mono)) {
		auto decl = ast->m_declaration;
		return static_cast<AST::MonoTypeHandle*>(decl->m_value);
	} else if (uf.is(meta_type, Tag::Func)) {
		auto decl = ast->m_declaration;
		return static_cast<AST::TypeFunctionHandle*>(decl->m_value);
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
	auto& uf = tc.core().m_meta_core;
	MetaTypeId meta_type = uf.eval(ast->m_meta_type);

	// TODO: support vars
	if (uf.is_ctor(meta_type))
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

static AST::TypeFunctionHandle* wrap_in_type_func_handle(
    AST::Expr* ast, TypeFunctionId value, AST::Allocator& alloc) {

	auto node = alloc.make<AST::TypeFunctionHandle>();
	node->m_value = value;
	node->m_syntax = ast;

	return node;
}

static AST::StructExpression* ct_eval(
    AST::StructExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	    ast->m_value = compute_type_func(ast, tc);
	    return ast;
}

static AST::TypeFunctionHandle* ct_eval(
    AST::UnionExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	return wrap_in_type_func_handle(ast, compute_type_func(ast, tc), alloc);
}

static AST::Constructor* constructor_from_ast(
    AST::Expr* ast, TypeChecker& tc, AST::Allocator& alloc) {
	auto& uf = tc.core().m_meta_core;
	MetaTypeId meta = uf.eval(ast->m_meta_type);
	auto constructor = alloc.make<AST::Constructor>();
	constructor->m_syntax = ast;

	if (uf.is(meta, Tag::Mono)) {
		constructor->m_mono = compute_mono(ast, tc);
	} else if (uf.is_ctor(meta)) {
		assert(ast->type() == ASTTag::AccessExpression);

		auto access = static_cast<AST::AccessExpression*>(ast);

		// dummy with one constructor, the one used
		std::unordered_map<InternedString, MonoId> structure;
		structure[access->m_member] = tc.new_var();
		TypeFunctionId dummy_tf = tc.core().new_type_function(
		    TypeFunctionTag::Variant, {}, std::move(structure), true);
		MonoId dummy_monotype =
		    tc.core().new_term(dummy_tf, {}, "Union Constructor Access");

		MonoId monotype = compute_mono(access->m_target, tc);

		tc.core().m_mono_core.unify(dummy_monotype, monotype);

		constructor->m_mono = monotype;
		constructor->m_id = access->m_member;
	} else {
		Log::fatal("Using something whose metatype is neither 'monotype' nor 'constructor' as a constructor");
	}

	return constructor;
}

static AST::MonoTypeHandle* ct_eval(
    AST::TypeTerm* ast, TypeChecker& tc, AST::Allocator& alloc) {

	MonoId result = compute_mono(ast, tc);

	auto handle = alloc.make<AST::MonoTypeHandle>();
	handle->m_value = result;
	handle->m_syntax = ast;
	return handle;
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

static void ct_visit(AST::Declaration* ast, TypeChecker& tc, AST::Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = ct_eval(ast->m_type_hint, tc, alloc);

	ast->m_value = ct_eval(ast->m_value, tc, alloc);
}

static TypeFunctionId get_foo(AST::Expr* ast) {
	switch (ast->type()) {

	case ASTTag::StructExpression: {
		auto struct_expression = static_cast<AST::StructExpression*>(ast);
		return struct_expression->m_value;
	}

	case ASTTag::UnionExpression: {
		auto union_expression = static_cast<AST::UnionExpression*>(ast);
		return union_expression->m_value;
		break;
	}

	case ASTTag::Identifier: {
		auto identifier = static_cast<AST::Identifier*>(ast);
		return get_foo(identifier->m_declaration->m_value);
		break;
	}
	
	case ASTTag::TypeFunctionHandle: {
		auto handle = static_cast<AST::TypeFunctionHandle*>(ast);
		return handle->m_value;
	}
	
	default:
		assert(0);
		break;
	}
}

static void foo(AST::Expr* ast, TypeChecker& tc) {
	switch (ast->type()) {

	case ASTTag::StructExpression: {
		auto struct_expression = static_cast<AST::StructExpression*>(ast);
		struct_expression->m_value = tc.core().new_type_function_var();
		break;
	}

	case ASTTag::UnionExpression: {
		auto union_expression = static_cast<AST::UnionExpression*>(ast);
		union_expression->m_value = tc.core().new_type_function_var();
		break;
	}

	case ASTTag::Identifier: {
		auto identifier = static_cast<AST::Identifier*>(ast);
		foo(identifier->m_declaration->m_value, tc);
		break;
	}
	
	default:
		assert(0);
		break;
	}
}

static void ct_visit(AST::Program* ast, TypeChecker& tc, AST::Allocator& alloc) {

	auto& uf = tc.core().m_meta_core;

	for (auto& decl : ast->m_declarations) {
		int meta_type = uf.eval(decl.m_meta_type);

#if 0
		std::cerr << "[ Pass 1 ] top level \"" << decl.m_identifier << "\"\n";
		std::cerr << "           metatype tag is: Tag(" << int(uf.tag(meta_type)) << ")\n";
#endif

		if (!uf.is_constant(meta_type))
			Log::fatal() << "Incomplete metatype inference on top level variable \"" << decl.m_identifier << "\"";

		// put a dummy var where required.
		if (uf.is(meta_type, Tag::Func)) {
			foo(decl.m_value, tc);
		} else if (uf.is(meta_type, Tag::Mono)) {
			auto handle = alloc.make<AST::MonoTypeHandle>();
			handle->m_value = tc.new_var(); // should it be hidden?
			handle->m_syntax = decl.m_value;
			decl.m_value = handle;
		}
	}

	auto const& comps = tc.declaration_order();
	for (auto const& decls : comps) {
		for (auto decl : decls) {
			int meta_type = uf.eval(decl->m_meta_type);

#if 0
			std::cerr << "[ Pass 2 ] top level \"" << decl->m_identifier << "\"\n";
			std::cerr << "           metatype tag is: Tag(" << int(uf.tag(meta_type)) << ")\n";
#endif

			if (uf.is(meta_type, Tag::Var))
				Log::fatal() << "Incomplete metatype inference on top level variable \"" << decl->m_identifier << "\"";

			if (uf.is(meta_type, Tag::Func)) {
				if (decl->m_type_hint)
					Log::fatal() << "type hint not allowed in typefunc declaration";

				TypeFunctionId tf = compute_type_func(decl->m_value, tc);
				tc.core().unify_type_function(tf, get_foo(decl->m_value));
			} else if (uf.is(meta_type, Tag::Mono)) {
				if (decl->m_type_hint)
					Log::fatal() << "type hint not allowed in type declaration";

				auto handle = static_cast<AST::MonoTypeHandle*>(decl->m_value);
				MonoId mt = compute_mono(handle->m_syntax, tc);
				tc.core().m_mono_core.unify(mt, handle->m_value);
			} else {
				if (decl->m_type_hint)
					decl->m_type_hint = ct_eval(decl->m_type_hint, tc, alloc);
				decl->m_value = ct_eval(decl->m_value, tc, alloc);
			}
		}
	}
}

static void ct_visit(AST::AST*& ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return ct_visit(static_cast<AST::type*>(ast), tc, alloc)

	switch (ast->type()) {
		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(Program);

	default:
		ast = ct_eval(ast, tc, alloc);
		return;
	}

#undef DISPATCH
}

static AST::Expr* ct_eval(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return ct_eval(static_cast<AST::type*>(ast), tc, alloc)

#define RETURN(type)                                                           \
	case ASTTag::type:                                                         \
		return static_cast<AST::type*>(ast);

#define REJECT(type)                                                           \
	case ASTTag::type:                                                         \
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

		REJECT(Block);
		REJECT(IfElseStatement);
		REJECT(WhileStatement);
		REJECT(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(Program);

		DISPATCH(TypeTerm);
		DISPATCH(StructExpression);
		DISPATCH(UnionExpression);
		REJECT(TypeFunctionHandle);
		REJECT(MonoTypeHandle);
		REJECT(Constructor);
	}

	Log::fatal() << "(internal) Unhandled case in ct_eval : "
	             << ast_string[int(ast->type())];

#undef DISPATCH
#undef RETURN
}

static std::unordered_map<InternedString, MonoId> build_map(
    std::vector<InternedString> const&, std::vector<AST::Expr*> const&, TypeChecker&);

static TypeFunctionId compute_type_func(AST::Identifier* ast, TypeChecker& tc) {
	assert(ast->m_declaration);

	auto& uf = tc.core().m_meta_core;
	MetaTypeId meta_type = uf.eval(ast->m_meta_type);
	assert(uf.is(meta_type, Tag::Func));

	auto decl = ast->m_declaration;
	return get_foo(decl->m_value);
}

static TypeFunctionId compute_type_func(AST::StructExpression* ast, TypeChecker& tc) {
	return tc.core().new_type_function(
	    TypeFunctionTag::Record,
	    ast->m_fields,
	    build_map(ast->m_fields, ast->m_types, tc));
}

static TypeFunctionId compute_type_func(AST::UnionExpression* ast, TypeChecker& tc) {
	return tc.core().new_type_function(
	    TypeFunctionTag::Variant,
	    {},
	    build_map(ast->m_constructors, ast->m_types, tc));
}

static TypeFunctionId compute_type_func(AST::Expr* ast, TypeChecker& tc) {
	assert(
	    ast->type() == ASTTag::Identifier ||
	    ast->type() == ASTTag::UnionExpression ||
	    ast->type() == ASTTag::StructExpression);

	if (ast->type() == ASTTag::UnionExpression) {
		return compute_type_func(static_cast<AST::UnionExpression*>(ast), tc);
	} else if (ast->type() == ASTTag::StructExpression) {
		return compute_type_func(static_cast<AST::StructExpression*>(ast), tc);
	} else {
		return compute_type_func(static_cast<AST::Identifier*>(ast), tc);
	}
}

static MonoId compute_mono(AST::Identifier* ast, TypeChecker& tc) {
	assert(ast->m_declaration);

	auto& uf = tc.core().m_meta_core;
	MetaTypeId meta_type = uf.eval(ast->m_meta_type);
	assert(uf.is(meta_type, Tag::Mono));

	auto decl = ast->m_declaration;
	assert(decl->m_value->type() == ASTTag::MonoTypeHandle);

	return static_cast<AST::MonoTypeHandle*>(decl->m_value)->m_value;
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
	assert(ast->type() == ASTTag::Identifier || ast->type() == ASTTag::TypeTerm);
	if (ast->type() == ASTTag::Identifier) {
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
