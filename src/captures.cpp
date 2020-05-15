#include "captures.hpp"

#include "ast_type.hpp"
#include "ast.hpp"

#include <cassert>

using Set = std::unordered_set<std::string>;

Set gather_captures(ASTFunctionLiteral* ast) {

	auto* body = dynamic_cast<ASTBlock*>(ast->m_body.get());
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
		auto decl = static_cast<ASTDeclaration*>(argp.get());
		internals.insert(decl->identifier_text());
	}

	for (auto& stmt : body->m_body) {
		// do not capture locals
		if (stmt->type() == ast_type::Declaration) {
			auto* decl = static_cast<ASTDeclaration*>(stmt.get());
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

Set gather_captures(ASTDeclaration* ast) {
	return gather_captures(ast->m_value.get());
}

Set gather_captures(ASTNumberLiteral* ast) {
	return {};
}

Set gather_captures(ASTStringLiteral* ast) {
	return {};
}

Set gather_captures(ASTIdentifier* ast) {
	return {{ ast->text() }};
}

Set gather_captures(ASTBinaryExpression* ast) {
	Set result = gather_captures(ast->m_lhs.get());
	Set secondary = gather_captures(ast->m_rhs.get());
	for(auto const& identifier : secondary)
		result.insert(identifier);
	return result;
}

Set gather_captures(ASTCallExpression* ast) {
	Set result = gather_captures(ast->m_callee.get());
	Set secondary = gather_captures(ast->m_args.get());
	for(auto const& identifier : secondary)
		result.insert(identifier);
	return result;
}

Set gather_captures(ASTIndexExpression* ast) {
	Set result = gather_captures(ast->m_callee.get());
	Set secondary = gather_captures(ast->m_index.get());
	for(auto const& identifier : secondary)
		result.insert(identifier);
	return result;
}

Set gather_captures(ASTArgumentList* ast) {
	Set result {};
	for(auto& child : ast->m_args){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(ASTBlock* ast) {
	Set result {};
	for(auto& child : ast->m_body){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(ASTReturnStatement* ast) {
	return gather_captures(ast->m_value.get());
}

Set gather_captures(ASTObjectLiteral* ast) {
	// TODO
	return {};
}

Set gather_captures(ASTArrayLiteral* ast) {
	Set result {};
	for(auto& child : ast->m_elements){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(ASTDictionaryLiteral* ast) {
	// TODO
	return {};
}

Set gather_captures(ASTDeclarationList* ast) {
	// TODO: figure out exactly what this should do
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		auto* child = static_cast<ASTDeclaration*>(decl.get());
		auto child_captures = gather_captures(child);
	}
	return {};
}

Set gather_captures(AST* ast) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return gather_captures(static_cast<ASTNumberLiteral*>(ast));
	case ast_type::StringLiteral:
		return gather_captures(static_cast<ASTStringLiteral*>(ast));
	case ast_type::ObjectLiteral:
		return gather_captures(static_cast<ASTObjectLiteral*>(ast));
	case ast_type::ArrayLiteral:
		return gather_captures(static_cast<ASTArrayLiteral*>(ast));
	case ast_type::DictionaryLiteral:
		return gather_captures(static_cast<ASTDictionaryLiteral*>(ast));
	case ast_type::FunctionLiteral:
		return gather_captures(static_cast<ASTFunctionLiteral*>(ast));
	case ast_type::DeclarationList:
		return gather_captures(static_cast<ASTDeclarationList*>(ast));
	case ast_type::Declaration:
		return gather_captures(static_cast<ASTDeclaration*>(ast));
	case ast_type::Identifier:
		return gather_captures(static_cast<ASTIdentifier*>(ast));
	case ast_type::BinaryExpression:
		return gather_captures(static_cast<ASTBinaryExpression*>(ast));
	case ast_type::CallExpression:
		return gather_captures(static_cast<ASTCallExpression*>(ast));
	case ast_type::IndexExpression:
		return gather_captures(static_cast<ASTIndexExpression*>(ast));
	case ast_type::ArgumentList:
		return gather_captures(static_cast<ASTArgumentList*>(ast));
	case ast_type::Block:
		return gather_captures(static_cast<ASTBlock*>(ast));
	case ast_type::ReturnStatement:
		return gather_captures(static_cast<ASTReturnStatement*>(ast));
	}

	return {};
}
