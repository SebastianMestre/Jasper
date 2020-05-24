#include "captures.hpp"

#include "ast_type.hpp"
#include "typed_ast.hpp"

#include <cassert>

using Set = std::unordered_set<std::string>;

Set gather_captures(TypedASTFunctionLiteral* ast) {

	auto* body = dynamic_cast<TypedASTBlock*>(ast->m_body.get());
	assert(body);

	Set internals;
	Set externals;

	auto scan_identifier = [&](std::string const& identifier) {
		if (internals.count(identifier) == 0) {
			externals.insert(identifier);
		}
	};

	// Do not capture own arguments
	for (auto& argp : ast->m_args) {
		assert(argp->type() == ast_type::Declaration);
		auto decl = static_cast<TypedASTDeclaration*>(argp.get());
		internals.insert(decl->identifier_text());
	}

	for (auto& stmt : body->m_body) {
		// do not capture locals
		if (stmt->type() == ast_type::Declaration) {
			auto* decl = static_cast<TypedASTDeclaration*>(stmt.get());
			internals.insert(decl->identifier_text());
		}

		Set child_captures = gather_captures(stmt.get());
		for (auto const& identifier : child_captures) {
			scan_identifier(identifier);
		}
	}

	assert(ast->m_captures.empty());
	ast->m_captures.reserve(externals.size());
	for(auto const& captured_identifier : externals)
		ast->m_captures.push_back(captured_identifier);

	return externals;
}

Set gather_captures(TypedASTDeclaration* ast) {
	return gather_captures(ast->m_value.get());
}

Set gather_captures(TypedASTNumberLiteral* ast) {
	return {};
}

Set gather_captures(TypedASTStringLiteral* ast) {
	return {};
}

Set gather_captures(TypedASTBooleanLiteral* ast) {
	return {};
}

Set gather_captures(TypedASTNullLiteral* ast) {
	return {};
}

Set gather_captures(TypedASTIdentifier* ast) {
	return {{ ast->text() }};
}

Set gather_captures(TypedASTBinaryExpression* ast) {
	Set result = gather_captures(ast->m_lhs.get());
	if(ast->m_op != token_type::DOT){
		Set secondary = gather_captures(ast->m_rhs.get());
		for(auto const& identifier : secondary)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedASTCallExpression* ast) {
	Set result = gather_captures(ast->m_callee.get());
	for(auto& child : ast->m_args){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedASTIndexExpression* ast) {
	Set result = gather_captures(ast->m_callee.get());
	Set secondary = gather_captures(ast->m_index.get());
	for(auto const& identifier : secondary)
		result.insert(identifier);
	return result;
}

Set gather_captures(TypedASTBlock* ast) {
	Set result {};
	for(auto& child : ast->m_body){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedASTReturnStatement* ast) {
	return gather_captures(ast->m_value.get());
}

Set gather_captures(TypedASTObjectLiteral* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedASTArrayLiteral* ast) {
	Set result {};
	for(auto& child : ast->m_elements){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedASTDictionaryLiteral* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedASTDeclarationList* ast) {
	// TODO: figure out exactly what this should do
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		auto* child = static_cast<TypedASTDeclaration*>(decl.get());
		auto child_captures = gather_captures(child);
	}
	return {};
}

Set gather_captures(TypedASTIfStatement* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedASTForStatement* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedAST* ast) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return gather_captures(static_cast<TypedASTNumberLiteral*>(ast));
	case ast_type::StringLiteral:
		return gather_captures(static_cast<TypedASTStringLiteral*>(ast));
	case ast_type::BooleanLiteral:
		return gather_captures(static_cast<TypedASTBooleanLiteral*>(ast));
	case ast_type::NullLiteral:
		return gather_captures(static_cast<TypedASTNullLiteral*>(ast));
	case ast_type::ObjectLiteral:
		return gather_captures(static_cast<TypedASTObjectLiteral*>(ast));
	case ast_type::ArrayLiteral:
		return gather_captures(static_cast<TypedASTArrayLiteral*>(ast));
	case ast_type::DictionaryLiteral:
		return gather_captures(static_cast<TypedASTDictionaryLiteral*>(ast));
	case ast_type::FunctionLiteral:
		return gather_captures(static_cast<TypedASTFunctionLiteral*>(ast));
	case ast_type::DeclarationList:
		return gather_captures(static_cast<TypedASTDeclarationList*>(ast));
	case ast_type::Declaration:
		return gather_captures(static_cast<TypedASTDeclaration*>(ast));
	case ast_type::Identifier:
		return gather_captures(static_cast<TypedASTIdentifier*>(ast));
	case ast_type::BinaryExpression:
		return gather_captures(static_cast<TypedASTBinaryExpression*>(ast));
	case ast_type::CallExpression:
		return gather_captures(static_cast<TypedASTCallExpression*>(ast));
	case ast_type::IndexExpression:
		return gather_captures(static_cast<TypedASTIndexExpression*>(ast));
	case ast_type::Block:
		return gather_captures(static_cast<TypedASTBlock*>(ast));
	case ast_type::ReturnStatement:
		return gather_captures(static_cast<TypedASTReturnStatement*>(ast));
	case ast_type::IfStatement:
		return gather_captures(static_cast<TypedASTIfStatement*>(ast));
	case ast_type::ForStatement:
		return gather_captures(static_cast<TypedASTForStatement*>(ast));
	}

	return {};
}
