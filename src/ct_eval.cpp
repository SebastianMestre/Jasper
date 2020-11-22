#include "ct_eval.hpp"

#include "typechecker.hpp"
#include "typed_ast.hpp"
#include "typed_ast_allocator.hpp"

#include <iostream>

#include <cassert>

namespace TypeChecker {

// literals

TypedAST::ArrayLiteral* ct_eval(
    TypedAST::ArrayLiteral* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	for (auto& element : ast->m_elements)
		element = ct_eval(element, tc, alloc);
	return ast;
}

// expressions

TypedAST::FunctionLiteral* ct_eval(
    TypedAST::FunctionLiteral* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {

	// TODO: type hints

	assert(ast->m_body->type() == TypedASTTag::Block);
	auto body = static_cast<TypedAST::Block*>(ast->m_body);
	for (auto& child : body->m_body)
		child = ct_eval(child, tc, alloc);

	return ast;
}

MonoId mono_type_from_ast(TypedAST::TypedAST* ast, TypeChecker& tc);
TypeFunctionId type_func_from_ast(TypedAST::TypedAST* ast, TypeChecker& tc);
TypedAST::Constructor* constructor_from_ast(
    TypedAST::TypedAST* ast, TypeChecker& tc, TypedAST::Allocator& alloc);

TypedAST::TypedAST* ct_eval(
    TypedAST::Identifier* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	// This does something very similar to what ct_eval(DeclarationList) does.
	// Maybe it can be de-duplicated?

	assert(ast);
	assert(ast->m_declaration);

	MetaTypeId meta_type = tc.m_core.m_meta_core.find(ast->m_meta_type);

	if (meta_type == tc.meta_value()) {
		return ast;
	} else if (meta_type == tc.meta_monotype()) {
		auto monotype = mono_type_from_ast(ast, tc);
		auto handle = alloc.make<TypedAST::MonoTypeHandle>();
		handle->m_value = monotype;
		handle->m_syntax = ast;
		return handle;
	} else if (meta_type == tc.meta_typefunc()) {
		auto type_func = type_func_from_ast(ast, tc);
		auto handle = alloc.make<TypedAST::TypeFunctionHandle>();
		handle->m_value = type_func;
		handle->m_syntax = ast;
		return handle;
	} else {
		// TODO: error
		return ast;
	}
}

TypedAST::CallExpression* ct_eval(
    TypedAST::CallExpression* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {

	ast->m_callee = ct_eval(ast->m_callee, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = ct_eval(arg, tc, alloc);

	return ast;
}

TypedAST::IndexExpression* ct_eval(
    TypedAST::IndexExpression* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_callee = ct_eval(ast->m_callee, tc, alloc);
	ast->m_index = ct_eval(ast->m_index, tc, alloc);

	return ast;
}

TypedAST::TernaryExpression* ct_eval(
    TypedAST::TernaryExpression* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_then_expr = ct_eval(ast->m_then_expr, tc, alloc);
	ast->m_else_expr = ct_eval(ast->m_else_expr, tc, alloc);
	return ast;
}

TypedAST::AccessExpression* ct_eval(
    TypedAST::AccessExpression* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_object = ct_eval(ast->m_object, tc, alloc);
	return ast;
}

TypedAST::ConstructorExpression* ct_eval(
    TypedAST::ConstructorExpression* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_constructor = constructor_from_ast(ast->m_constructor, tc, alloc);

	for (auto& arg : ast->m_args)
		arg = ct_eval(arg, tc, alloc);

	return ast;
}

// statements

TypedAST::Block* ct_eval(
    TypedAST::Block* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	for (auto& child : ast->m_body)
		child = ct_eval(child, tc, alloc);
	return ast;
}

TypedAST::IfElseStatement* ct_eval(
    TypedAST::IfElseStatement* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_body = ct_eval(ast->m_body, tc, alloc);
	if (ast->m_else_body)
		ast->m_else_body = ct_eval(ast->m_else_body, tc, alloc);
	return ast;
}

TypedAST::ForStatement* ct_eval(
    TypedAST::ForStatement* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ct_eval(&ast->m_declaration, tc, alloc);
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_action = ct_eval(ast->m_action, tc, alloc);
	ast->m_body = ct_eval(ast->m_body, tc, alloc);
	return ast;
}

TypedAST::WhileStatement* ct_eval(
    TypedAST::WhileStatement* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_condition = ct_eval(ast->m_condition, tc, alloc);
	ast->m_body = ct_eval(ast->m_body, tc, alloc);
	return ast;
}

TypedAST::ReturnStatement* ct_eval(
    TypedAST::ReturnStatement* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	ast->m_value = ct_eval(ast->m_value, tc, alloc);
	return ast;
}

// types

TypeFunctionId type_func_from_ast(TypedAST::TypedAST* ast, TypeChecker& tc) {
	assert(tc.m_core.m_meta_core.find(ast->m_meta_type) == tc.meta_typefunc());
	if (ast->type() == TypedASTTag::TypeFunctionHandle) {
		return static_cast<TypedAST::TypeFunctionHandle*>(ast)->m_value;
	} else if (ast->type() == TypedASTTag::Identifier) {
		auto as_id = static_cast<TypedAST::Identifier*>(ast);
		auto decl = as_id->m_declaration;
		auto value =
		    static_cast<TypedAST::TypeFunctionHandle*>(decl->m_value);
		return value->m_value;
	// TODO: handle duplication better
	} else if (ast->type() == TypedASTTag::UnionExpression) {
		auto as_ue = static_cast<TypedAST::UnionExpression*>(ast);

		std::vector<InternedString> constructors;
		std::unordered_map<InternedString, MonoId> structure;
		int constructor_count = as_ue->m_constructors.size();
		for (int i = 0; i < constructor_count; ++i){
			MonoId mono = mono_type_from_ast(as_ue->m_types[i], tc);
			InternedString name = as_ue->m_constructors[i].text();
			assert(!structure.count(name));
			structure[name] = mono;
			constructors.push_back(name);
		}

		TypeFunctionId result = tc.m_core.new_type_function(
		    TypeFunctionTag::Sum, std::move(constructors), std::move(structure));

		return result;
	} else if (ast->type() == TypedASTTag::StructExpression) {
		auto as_se = static_cast<TypedAST::StructExpression*>(ast);

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

MonoId mono_type_from_ast(TypedAST::TypedAST* ast, TypeChecker& tc){
	assert(tc.m_core.m_meta_core.find(ast->m_meta_type) == tc.meta_monotype());
	if (ast->type() == TypedASTTag::MonoTypeHandle) {
		return static_cast<TypedAST::MonoTypeHandle*>(ast)->m_value;
	} else if (ast->type() == TypedASTTag::Identifier) {
		auto as_id = static_cast<TypedAST::Identifier*>(ast);
		auto decl = as_id->m_declaration;
		auto value = static_cast<TypedAST::MonoTypeHandle*>(decl->m_value);

		return value->m_value;
	} else if (ast->type() == TypedASTTag::TypeTerm) {
		auto as_tt = static_cast<TypedAST::TypeTerm*>(ast);

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

TypedAST::Constructor* constructor_from_ast(
    TypedAST::TypedAST* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	MetaTypeId meta = tc.m_core.m_meta_core.find(ast->m_meta_type);
	auto constructor = alloc.make<TypedAST::Constructor>();
	constructor->m_syntax = ast;

	if (meta == tc.meta_monotype()) {
		constructor->m_mono = mono_type_from_ast(ast, tc);
	} else if (meta == tc.meta_constructor()) {
		assert(ast->type() == TypedASTTag::AccessExpression);

		auto access = static_cast<TypedAST::AccessExpression*>(ast);
		constructor->m_mono = mono_type_from_ast(access->m_object, tc);
		constructor->m_id = access->m_member;
	} else {
		assert(0 && "wrong constructor metatype");
	}

	return constructor;
}

// declarations

TypedAST::Declaration* ct_eval(
    TypedAST::Declaration* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	if (ast->m_type_hint)
		ast->m_type_hint = ct_eval(ast->m_type_hint, tc, alloc);

	ast->m_value = ct_eval(ast->m_value, tc, alloc);
	return ast;
}

TypedAST::DeclarationList* ct_eval(
    TypedAST::DeclarationList* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	// TODO: we might need to do the SCC decomposition here, too.

	for (auto& decl : ast->m_declarations) {
		int meta_type = tc.m_core.m_meta_core.find(decl.m_meta_type);
		// put a dummy var where required.
		if (meta_type == tc.meta_typefunc()) {
			auto handle = alloc.make<TypedAST::TypeFunctionHandle>();
			handle->m_value = tc.m_core.m_tf_core.new_var();
			handle->m_syntax = decl.m_value;
			decl.m_value = handle;
		} else if (meta_type == tc.meta_monotype()) {
			auto handle = alloc.make<TypedAST::MonoTypeHandle>();
			handle->m_value = tc.new_var(); // should it be hidden?
			handle->m_syntax = decl.m_value;
			decl.m_value = handle;
		}
	}

	for (auto& decl : ast->m_declarations) {
		int meta_type = tc.m_core.m_meta_core.find(decl.m_meta_type);
		if (meta_type == tc.meta_typefunc()) {
			assert(!decl.m_type_hint && "type hint not allowed in type function declaration");
			auto handle =
			    static_cast<TypedAST::TypeFunctionHandle*>(decl.m_value);

			TypeFunctionId tf = type_func_from_ast(handle->m_syntax, tc);
			tc.m_core.m_tf_core.unify(tf, handle->m_value);
		} else if (meta_type == tc.meta_monotype()) {
			assert(!decl.m_type_hint && "type hint not allowed in monotype declaration");
			auto handle =
			    static_cast<TypedAST::MonoTypeHandle*>(decl.m_value);

			MonoId mt = mono_type_from_ast(handle->m_syntax, tc);
			tc.m_core.m_mono_core.unify(mt, handle->m_value);
		} else {
			if (decl.m_type_hint)
				decl.m_type_hint = ct_eval(decl.m_type_hint, tc, alloc);
			decl.m_value = ct_eval(decl.m_value, tc, alloc);
		}
	}

	return ast;
}

TypedAST::MonoTypeHandle* ct_eval(
    TypedAST::TypeTerm* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
	auto handle = alloc.make<TypedAST::MonoTypeHandle>();
	handle->m_value = mono_type_from_ast(ast, tc);
	handle->m_syntax = ast;
	return handle;
}

TypedAST::TypedAST* ct_eval(
    TypedAST::TypedAST* ast, TypeChecker& tc, TypedAST::Allocator& alloc) {
#define DISPATCH(type)                                                         \
	case TypedASTTag::type:                                                    \
		return ct_eval(static_cast<TypedAST::type*>(ast), tc, alloc)           \

#define RETURN(type)                                                           \
	case TypedASTTag::type:                                                    \
		return ast;

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
		DISPATCH(ConstructorExpression);

		DISPATCH(Block);
		DISPATCH(IfElseStatement);
		DISPATCH(ForStatement);
		DISPATCH(WhileStatement);
		DISPATCH(ReturnStatement);

		DISPATCH(Declaration);
		DISPATCH(DeclarationList);

		DISPATCH(TypeTerm);
	}

	std::cerr << "Unhandled case in " << __PRETTY_FUNCTION__ << " : "
	          << typed_ast_string[int(ast->type())] << "\n";
	assert(0);

#undef DISPATCH
#undef RETURN
}

} // namespace TypeChecker
