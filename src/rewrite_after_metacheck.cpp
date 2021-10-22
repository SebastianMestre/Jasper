#include "rewrite_after_metacheck.hpp"

#include "./log/log.hpp"
#include "ast.hpp"
#include "ast_allocator.hpp"
#include "typechecker.hpp"

#include <cassert>
#include <iostream>

namespace TypeChecker {

static AST::Expr* rewrite(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc);
static void rewrite_inplace(AST::AST*& ast, TypeChecker& tc, AST::Allocator& alloc);

static AST::Expr* rewrite(
    AST::AccessExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	ast->m_target = rewrite(ast->m_target, tc, alloc);

	auto& uf = tc.m_core.m_meta_core;
	MetaTypeId meta_type = uf.eval(ast->m_meta_type);

	if (uf.is(meta_type, Tag::Ctor)) {
		auto node = alloc.make<AST::UnionAccessExpression>();
		node->m_meta_type = meta_type;
		node->m_member = ast->m_member;
		node->m_target = ast->m_target;
		return node;
	}

	return ast;
}

static AST::Expr* rewrite(
    AST::ConstructorExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	auto* target = rewrite(ast->m_constructor, tc, alloc);

	auto& uf = tc.m_core.m_meta_core;
	MetaTypeId meta = uf.eval(target->m_meta_type);

	if (uf.is(meta, Tag::Mono)) {
		auto result = alloc.make<AST::StructConstruction>();
		result->m_meta_type = uf.make_const_node(Tag::Term);
		for (auto arg : ast->m_args)
			result->m_args.push_back(rewrite(arg, tc, alloc));
		result->m_constructor = target;
		return result;
	} else if (uf.is(meta, Tag::Ctor)) {
		auto result = alloc.make<AST::UnionConstruction>();
		result->m_meta_type = uf.make_const_node(Tag::Term);
		assert(ast->m_args.size() == 1);
		result->m_arg = rewrite(ast->m_args[0], tc, alloc);
		result->m_constructor = target;
		return result;
	} else {
		Log::fatal("Using something whose metatype is neither 'monotype' nor 'constructor' as a constructor");
	}
}



// literals
static AST::ArrayLiteral* rewrite(
    AST::ArrayLiteral* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& element : ast->m_elements)
		element = rewrite(element, tc, alloc);
	return ast;
}

// expressions

static AST::FunctionLiteral* rewrite(
    AST::FunctionLiteral* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& arg : ast->m_args)
		if (arg.m_type_hint)
			arg.m_type_hint = rewrite(arg.m_type_hint, tc, alloc);

	ast->m_body = rewrite(ast->m_body, tc, alloc);

	return ast;
}

static AST::Expr* rewrite(
    AST::Identifier* ast, TypeChecker& tc, AST::Allocator& alloc) {
	// This does something very similar to what rewrite(Program) does.
	// Maybe it can be de-duplicated?

	assert(ast);
	assert(ast->m_declaration);

	auto& uf = tc.m_core.m_meta_core;
	MetaTypeId meta_type = uf.eval(ast->m_meta_type);

	if (!uf.is_constant(meta_type))
		Log::fatal() << "Incomplete type inference on identifier" << ast->text();

	if (uf.is(meta_type, Tag::Term)) {
		return ast;
	} else if (uf.is(meta_type, Tag::Mono)) {
		auto decl = ast->m_declaration;
		return decl->m_value;
	} else if (uf.is(meta_type, Tag::Func)) {
		auto decl = ast->m_declaration;
		return decl->m_value;
	}

	assert(0 && "UNREACHABLE");
}

static AST::CallExpression* rewrite(
    AST::CallExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	ast->m_callee = rewrite(ast->m_callee, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = rewrite(arg, tc, alloc);

	return ast;
}

static AST::IndexExpression* rewrite(
    AST::IndexExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_callee = rewrite(ast->m_callee, tc, alloc);
	ast->m_index = rewrite(ast->m_index, tc, alloc);

	return ast;
}

static AST::TernaryExpression* rewrite(
    AST::TernaryExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = rewrite(ast->m_condition, tc, alloc);
	ast->m_then_expr = rewrite(ast->m_then_expr, tc, alloc);
	ast->m_else_expr = rewrite(ast->m_else_expr, tc, alloc);
	return ast;
}

static AST::Expr* rewrite(
    AST::MatchExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	// NOTE: no need to rewrite the identifier, because it is guaranteed
	// to be of metatype value, thus nothing needs to be done

	if (ast->m_type_hint) {
		ast->m_type_hint = rewrite(ast->m_type_hint, tc, alloc);
	}

	for (auto& kv : ast->m_cases) {
		auto& case_data = kv.second;

		// TODO(SMestre): factor this out (it's repeated in a bunch of other places)
		if (case_data.m_declaration.m_type_hint) {
			case_data.m_declaration.m_type_hint =
			    rewrite(case_data.m_declaration.m_type_hint, tc, alloc);
		}

		case_data.m_expression = rewrite(case_data.m_expression, tc, alloc);
	}

	return ast;
}

static void rewrite_inplace(AST::Block* ast, TypeChecker& tc, AST::Allocator& alloc);
static AST::Expr* rewrite(
    AST::SequenceExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	rewrite_inplace(ast->m_body, tc, alloc);
	return ast;
}
// types


static AST::StructExpression* rewrite(
    AST::StructExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	return ast;
}

static AST::UnionExpression* rewrite(
    AST::UnionExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	return ast;
}

static AST::TypeTerm* rewrite(
    AST::TypeTerm* ast, TypeChecker& tc, AST::Allocator& alloc) {

	return ast;
}

// statements

static void rewrite_inplace(AST::Block* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& child : ast->m_body) {
		rewrite_inplace(child, tc, alloc);
	}
}

static void rewrite_inplace(AST::IfElseStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = rewrite(ast->m_condition, tc, alloc);
	rewrite_inplace(ast->m_body, tc, alloc);
	if (ast->m_else_body) {
		rewrite_inplace(ast->m_else_body, tc, alloc);
	}
}

static void rewrite_inplace(AST::WhileStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = rewrite(ast->m_condition, tc, alloc);
	rewrite_inplace(ast->m_body, tc, alloc);
}

static void rewrite_inplace(AST::ReturnStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_value = rewrite(ast->m_value, tc, alloc);
}

static void rewrite_inplace(AST::Declaration* ast, TypeChecker& tc, AST::Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = rewrite(ast->m_type_hint, tc, alloc);

	ast->m_value = rewrite(ast->m_value, tc, alloc);
}

static void rewrite_inplace(AST::Program* ast, TypeChecker& tc, AST::Allocator& alloc) {

	auto& uf = tc.m_core.m_meta_core;

	auto const& comps = tc.m_env.declaration_components;
	for (auto const& decls : comps) {
		for (auto decl : decls) {
			int meta_type = uf.eval(decl->m_meta_type);

#if 0
			std::cerr << "[ Pass 2 ] top level \"" << decl->m_identifier << "\"\n";
			std::cerr << "           metatype tag is: Tag(" << int(uf.tag(meta_type)) << ")\n";
#endif

			if (uf.is(meta_type, Tag::Var))
				Log::fatal() << "Incomplete metatype inference on top level variable \"" << decl->m_identifier << "\"";

			if (decl->m_type_hint) {
				if (uf.is(meta_type, Tag::Func)) {
					Log::fatal() << "type hint not allowed in typefunc declaration";
				} else if (uf.is(meta_type, Tag::Mono)) {
					Log::fatal() << "type hint not allowed in type declaration";
				} else {
					decl->m_type_hint = rewrite(decl->m_type_hint, tc, alloc);
				}
			}

			decl->m_value = rewrite(decl->m_value, tc, alloc);
		}
	}
}

static void rewrite_inplace(AST::AST*& ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return rewrite_inplace(static_cast<AST::type*>(ast), tc, alloc)

	switch (ast->type()) {
		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(Program);

	default:
		ast = rewrite(ast, tc, alloc);
		return;
	}

#undef DISPATCH
}

static AST::Expr* rewrite(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return rewrite(static_cast<AST::type*>(ast), tc, alloc)

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
		DISPATCH(SequenceExpression);
		DISPATCH(ConstructorExpression);

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
	}

	Log::fatal() << "(internal) Unhandled case in rewrite : "
	             << ast_string[int(ast->type())];

#undef DISPATCH
#undef RETURN
}

void rewrite_after_metacheck(AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc) {
	rewrite_inplace(ast, tc, alloc);
}

} // namespace TypeChecker
