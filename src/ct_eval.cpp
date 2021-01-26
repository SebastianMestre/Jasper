#include "ct_eval.hpp"

#include "./log/log.hpp"
#include "typechecker.hpp"
#include "ast.hpp"
#include "ast_allocator.hpp"

#include <sstream>

#include <cassert>

namespace TypeChecker {

// literals

AST::ArrayLiteral* ct_eval(
    AST::ArrayLiteral* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& element : ast->m_elements)
		element = ct_eval(element, tc, alloc);
	return ast;
}

// expressions

AST::FunctionLiteral* ct_eval(
    AST::FunctionLiteral* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& arg : ast->m_args)
		if (arg.m_type_hint)
			arg.m_type_hint = ct_eval(arg.m_type_hint, tc, alloc);

	ast->m_body = ct_eval(ast->m_body, tc, alloc);

	return ast;
}

MonoId mono_type_from_ast(AST::AST* ast, TypeChecker& tc);
TypeFunctionId type_func_from_ast(AST::AST* ast, TypeChecker& tc);
AST::Constructor* constructor_from_ast(
    AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc);

AST::AST* ct_eval(
    AST::Identifier* ast, TypeChecker& tc, AST::Allocator& alloc) {
	// This does something very similar to what ct_eval(DeclarationList) does.
	// Maybe it can be de-duplicated?

	assert(ast);
	assert(ast->m_declaration);

	MetaTypeId meta_type = tc.m_core.m_meta_core.find(ast->m_meta_type);

	if (meta_type == tc.meta_value()) {
		return ast;
	} else if (meta_type == tc.meta_monotype()) {
		auto monotype = mono_type_from_ast(ast, tc);
		auto handle = alloc.make<AST::MonoTypeHandle>();
		handle->m_value = monotype;
		handle->m_syntax = ast;
		return handle;
	} else if (meta_type == tc.meta_typefunc()) {
		auto type_func = type_func_from_ast(ast, tc);
		auto handle = alloc.make<AST::TypeFunctionHandle>();
		handle->m_value = type_func;
		handle->m_syntax = ast;
		return handle;
	} else {
		// TODO: error
		return ast;
	}
}

AST::CallExpression* ct_eval(
    AST::CallExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {

	ast->m_callee = ct_eval(ast->m_callee, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = ct_eval(arg, tc, alloc);

	return ast;
}

AST::IndexExpression* ct_eval(
    AST::IndexExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_callee = ct_eval(ast->m_callee, tc, alloc);
	ast->m_index = ct_eval(ast->m_index, tc, alloc);

	return ast;
}

AST::TernaryExpression* ct_eval(
    AST::TernaryExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_then_expr = ct_eval(ast->m_then_expr, tc, alloc);
	ast->m_else_expr = ct_eval(ast->m_else_expr, tc, alloc);
	return ast;
}

AST::AST* ct_eval(
    AST::AccessExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	MetaTypeId metatype = tc.m_core.m_meta_core.find(ast->m_meta_type);
	// TODO: support vars
	if (metatype == tc.meta_constructor())
		return constructor_from_ast(ast, tc, alloc);

	ast->m_record = ct_eval(ast->m_record, tc, alloc);
	return ast;
}

AST::AST* ct_eval(
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

AST::ConstructorExpression* ct_eval(
    AST::ConstructorExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_constructor = constructor_from_ast(ast->m_constructor, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = ct_eval(arg, tc, alloc);

	return ast;
}

AST::AST* ct_eval(
    AST::SequenceExpression* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_body = static_cast<AST::Block*>(ct_eval(ast->m_body, tc, alloc));
	return ast;
}

// statements

AST::Block* ct_eval(
    AST::Block* ast, TypeChecker& tc, AST::Allocator& alloc) {
	for (auto& child : ast->m_body)
		child = ct_eval(child, tc, alloc);
	return ast;
}

AST::IfElseStatement* ct_eval(
    AST::IfElseStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_body = ct_eval(ast->m_body, tc, alloc);
	if (ast->m_else_body)
		ast->m_else_body = ct_eval(ast->m_else_body, tc, alloc);
	return ast;
}

AST::ForStatement* ct_eval(
    AST::ForStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ct_eval(&ast->m_declaration, tc, alloc);
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_action = ct_eval(ast->m_action, tc, alloc);
	ast->m_body = ct_eval(ast->m_body, tc, alloc);
	return ast;
}

AST::WhileStatement* ct_eval(
    AST::WhileStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_body = ct_eval(ast->m_body, tc, alloc);
	return ast;
}

AST::ReturnStatement* ct_eval(
    AST::ReturnStatement* ast, TypeChecker& tc, AST::Allocator& alloc) {
	ast->m_value = ct_eval(ast->m_value, tc, alloc);
	return ast;
}

// types

TypeFunctionId type_func_from_ast(AST::AST* ast, TypeChecker& tc) {
	assert(tc.m_core.m_meta_core.find(ast->m_meta_type) == tc.meta_typefunc());
	if (ast->type() == ASTTag::TypeFunctionHandle) {
		return static_cast<AST::TypeFunctionHandle*>(ast)->m_value;
	} else if (ast->type() == ASTTag::Identifier) {
		auto as_id = static_cast<AST::Identifier*>(ast);
		auto decl = as_id->m_declaration;
		auto value =
		    static_cast<AST::TypeFunctionHandle*>(decl->m_value);
		return value->m_value;
	// TODO: handle duplication better
	} else if (ast->type() == ASTTag::UnionExpression) {
		auto as_ue = static_cast<AST::UnionExpression*>(ast);

		std::unordered_map<InternedString, MonoId> structure;
		int constructor_count = as_ue->m_constructors.size();
		for (int i = 0; i < constructor_count; ++i){
			MonoId mono = mono_type_from_ast(as_ue->m_types[i], tc);
			InternedString name = as_ue->m_constructors[i].text();
			assert(!structure.count(name));
			structure[name] = mono;
		}

		TypeFunctionId result = tc.m_core.new_type_function(
		    TypeFunctionTag::Variant, {}, std::move(structure));

		return result;
	} else if (ast->type() == ASTTag::StructExpression) {
		auto as_se = static_cast<AST::StructExpression*>(ast);

		std::vector<InternedString> fields;
		std::unordered_map<InternedString, MonoId> structure;
		int field_count = as_se->m_fields.size();
		for (int i = 0; i < field_count; ++i){
			MonoId mono = mono_type_from_ast(as_se->m_types[i], tc);
			InternedString name = as_se->m_fields[i].text();
			assert(!structure.count(name));
			structure[name] = mono;
			fields.push_back(name);
		}

		TypeFunctionId result = tc.m_core.new_type_function(
		    TypeFunctionTag::Record, std::move(fields), std::move(structure));

		return result;
	} else {
		assert(0);
	}
}

MonoId mono_type_from_ast(AST::AST* ast, TypeChecker& tc){
	assert(tc.m_core.m_meta_core.find(ast->m_meta_type) == tc.meta_monotype());
	if (ast->type() == ASTTag::MonoTypeHandle) {
		return static_cast<AST::MonoTypeHandle*>(ast)->m_value;
	} else if (ast->type() == ASTTag::Identifier) {
		auto as_id = static_cast<AST::Identifier*>(ast);
		auto decl = as_id->m_declaration;
		auto value = static_cast<AST::MonoTypeHandle*>(decl->m_value);

		return value->m_value;
	} else if (ast->type() == ASTTag::TypeTerm) {
		auto as_tt = static_cast<AST::TypeTerm*>(ast);

		TypeFunctionId type_function = type_func_from_ast(as_tt->m_callee, tc);

		std::vector<MonoId> args;
		for (auto& arg : as_tt->m_args)
			args.push_back(mono_type_from_ast(arg, tc));

		MonoId result = tc.m_core.new_term(type_function, std::move(args), "from ast");
		return result;
	} else {
		assert(0);
	}
}

AST::Constructor* constructor_from_ast(
    AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc) {
	MetaTypeId meta = tc.m_core.m_meta_core.find(ast->m_meta_type);
	auto constructor = alloc.make<AST::Constructor>();
	constructor->m_syntax = ast;

	if (meta == tc.meta_monotype()) {
		constructor->m_mono = mono_type_from_ast(ast, tc);
	} else if (meta == tc.meta_constructor()) {
		assert(ast->type() == ASTTag::AccessExpression);

		auto access = static_cast<AST::AccessExpression*>(ast);

		// dummy with one constructor, the one used
		std::unordered_map<InternedString, MonoId> structure;
		structure[access->m_member] = tc.new_var();
		TypeFunctionId dummy_tf = tc.m_core.new_type_function(
		    TypeFunctionTag::Variant, {}, std::move(structure), true);
		MonoId dummy_monotype =
		    tc.m_core.new_term(dummy_tf, {}, "Union Constructor Access");

		MonoId monotype = mono_type_from_ast(access->m_record, tc);

		tc.m_core.m_mono_core.unify(dummy_monotype, monotype);

		constructor->m_mono = monotype;
		constructor->m_id = access->m_member;
	} else {
		Log::fatal("Using something whose metatype is neither 'monotype' nor 'constructor' as a constructor");
	}

	return constructor;
}

// declarations

AST::Declaration* ct_eval(
    AST::Declaration* ast, TypeChecker& tc, AST::Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = ct_eval(ast->m_type_hint, tc, alloc);

	ast->m_value = ct_eval(ast->m_value, tc, alloc);
	return ast;
}

AST::DeclarationList* ct_eval(
    AST::DeclarationList* ast, TypeChecker& tc, AST::Allocator& alloc) {

	for (auto& decl : ast->m_declarations) {
		int meta_type = tc.m_core.m_meta_core.find(decl.m_meta_type);
		// put a dummy var where required.
		if (meta_type == tc.meta_typefunc()) {
			auto handle = alloc.make<AST::TypeFunctionHandle>();
			handle->m_value = tc.m_core.m_tf_core.new_var();
			handle->m_syntax = decl.m_value;
			decl.m_value = handle;
		} else if (meta_type == tc.meta_monotype()) {
			auto handle = alloc.make<AST::MonoTypeHandle>();
			handle->m_value = tc.new_var(); // should it be hidden?
			handle->m_syntax = decl.m_value;
			decl.m_value = handle;
		}
	}

	auto const& comps = tc.m_env.declaration_components;
	for (auto const& decls : comps) {
		for (auto decl : decls) {
			int meta_type = tc.m_core.m_meta_core.find(decl->m_meta_type);
			if (meta_type == tc.meta_typefunc()) {
				assert(!decl->m_type_hint && "type hint not allowed in type function declaration");
				auto handle =
					static_cast<AST::TypeFunctionHandle*>(decl->m_value);

				TypeFunctionId tf = type_func_from_ast(handle->m_syntax, tc);
				tc.m_core.m_tf_core.unify(tf, handle->m_value);
			} else if (meta_type == tc.meta_monotype()) {
				assert(!decl->m_type_hint && "type hint not allowed in monotype declaration");
				auto handle =
					static_cast<AST::MonoTypeHandle*>(decl->m_value);

				MonoId mt = mono_type_from_ast(handle->m_syntax, tc);
				tc.m_core.m_mono_core.unify(mt, handle->m_value);
			} else {
				if (decl->m_type_hint)
					decl->m_type_hint = ct_eval(decl->m_type_hint, tc, alloc);
				decl->m_value = ct_eval(decl->m_value, tc, alloc);
			}
		}
	}

	return ast;
}

AST::MonoTypeHandle* ct_eval(
    AST::TypeTerm* ast, TypeChecker& tc, AST::Allocator& alloc) {
	auto handle = alloc.make<AST::MonoTypeHandle>();
	handle->m_value = mono_type_from_ast(ast, tc);
	handle->m_syntax = ast;
	return handle;
}

AST::AST* ct_eval(
    AST::AST* ast, TypeChecker& tc, AST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case ASTTag::type:                                                         \
		return ct_eval(static_cast<AST::type*>(ast), tc, alloc)

#define RETURN(type)                                                           \
	case ASTTag::type:                                                         \
		return ast;

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

		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(TypeTerm);
		REJECT(StructExpression); // handled in type_func_from_ast
		REJECT(UnionExpression);  // handled in type_func_from_ast
		REJECT(TypeFunctionHandle);
		REJECT(MonoTypeHandle);
		REJECT(Constructor);
	}

	Log::fatal() << "(internal) Unhandled case in ct_eval : "
	             << ast_string[int(ast->type())];

#undef DISPATCH
#undef RETURN
}

} // namespace TypeChecker
