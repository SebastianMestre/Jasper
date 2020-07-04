#include "captures.hpp"

#include "ast_type.hpp"
#include "typed_ast.hpp"

#include <cassert>

using Set = std::unordered_set<std::string>;

Set gather_captures(TypedAST::FunctionLiteral* ast) {

	auto* body = dynamic_cast<TypedAST::Block*>(ast->m_body.get());
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
		auto decl = static_cast<TypedAST::Declaration*>(argp.get());
		internals.insert(decl->identifier_text());
	}

	for (auto& stmt : body->m_body) {
		// do not capture locals
		if (stmt->type() == ast_type::Declaration) {
			auto* decl = static_cast<TypedAST::Declaration*>(stmt.get());
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

Set gather_captures(TypedAST::Declaration* ast) {
	return gather_captures(ast->m_value.get());
}

Set gather_captures(TypedAST::NumberLiteral* ast) {
	return {};
}

Set gather_captures(TypedAST::StringLiteral* ast) {
	return {};
}

Set gather_captures(TypedAST::BooleanLiteral* ast) {
	return {};
}

Set gather_captures(TypedAST::NullLiteral* ast) {
	return {};
}

Set gather_captures(TypedAST::Identifier* ast) {
	return {{ ast->text() }};
}

Set gather_captures(TypedAST::CallExpression* ast) {
	Set result = gather_captures(ast->m_callee.get());
	for(auto& child : ast->m_args){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedAST::IndexExpression* ast) {
	Set result = gather_captures(ast->m_callee.get());
	Set secondary = gather_captures(ast->m_index.get());
	for(auto const& identifier : secondary)
		result.insert(identifier);
	return result;
}

Set gather_captures(TypedAST::Block* ast) {
	Set result {};
	for(auto& child : ast->m_body){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedAST::ReturnStatement* ast) {
	return gather_captures(ast->m_value.get());
}

Set gather_captures(TypedAST::ObjectLiteral* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedAST::ArrayLiteral* ast) {
	Set result {};
	for(auto& child : ast->m_elements){
		Set child_captures = gather_captures(child.get());
		for(auto const& identifier : child_captures)
			result.insert(identifier);
	}
	return result;
}

Set gather_captures(TypedAST::DictionaryLiteral* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedAST::DeclarationList* ast) {
	// TODO: figure out exactly what this should do
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		auto* child = static_cast<TypedAST::Declaration*>(decl.get());
		auto child_captures = gather_captures(child);
	}
	return {};
}

Set gather_captures(TypedAST::IfStatement* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedAST::ForStatement* ast) {
	// TODO
	return {};
}

Set gather_captures(TypedAST::TypedAST* ast) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return gather_captures(static_cast<TypedAST::NumberLiteral*>(ast));
	case ast_type::StringLiteral:
		return gather_captures(static_cast<TypedAST::StringLiteral*>(ast));
	case ast_type::BooleanLiteral:
		return gather_captures(static_cast<TypedAST::BooleanLiteral*>(ast));
	case ast_type::NullLiteral:
		return gather_captures(static_cast<TypedAST::NullLiteral*>(ast));
	case ast_type::ObjectLiteral:
		return gather_captures(static_cast<TypedAST::ObjectLiteral*>(ast));
	case ast_type::ArrayLiteral:
		return gather_captures(static_cast<TypedAST::ArrayLiteral*>(ast));
	case ast_type::DictionaryLiteral:
		return gather_captures(static_cast<TypedAST::DictionaryLiteral*>(ast));
	case ast_type::FunctionLiteral:
		return gather_captures(static_cast<TypedAST::FunctionLiteral*>(ast));
	case ast_type::DeclarationList:
		return gather_captures(static_cast<TypedAST::DeclarationList*>(ast));
	case ast_type::Declaration:
		return gather_captures(static_cast<TypedAST::Declaration*>(ast));
	case ast_type::Identifier:
		return gather_captures(static_cast<TypedAST::Identifier*>(ast));
	case ast_type::CallExpression:
		return gather_captures(static_cast<TypedAST::CallExpression*>(ast));
	case ast_type::IndexExpression:
		return gather_captures(static_cast<TypedAST::IndexExpression*>(ast));
	case ast_type::Block:
		return gather_captures(static_cast<TypedAST::Block*>(ast));
	case ast_type::ReturnStatement:
		return gather_captures(static_cast<TypedAST::ReturnStatement*>(ast));
	case ast_type::IfStatement:
		return gather_captures(static_cast<TypedAST::IfStatement*>(ast));
	case ast_type::ForStatement:
		return gather_captures(static_cast<TypedAST::ForStatement*>(ast));
	case ast_type::BinaryExpression:
		assert(0);
	}

	return {};
}
