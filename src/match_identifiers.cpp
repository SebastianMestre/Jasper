#include "match_identifiers.hpp"

#include <unordered_map>

#include "typed_ast.hpp"

#include <cassert>

namespace TypeChecker {

struct Scope {
	bool m_nested { false };
	std::unordered_map<std::string, TypedASTDeclaration*> m_vars;
};

struct FakeEnvironment {
	std::vector<Scope> m_scopes;

	FakeEnvironment();

	void new_scope();
	void new_nested_scope();
	void end_scope();

	void declare_name(std::string const&, TypedASTDeclaration*);
	TypedASTDeclaration* access_name(std::string const&);

	void match_identifiers(TypedAST*);
	void match_identifiers(TypedASTDeclaration*);
	void match_identifiers(TypedASTIdentifier*);
	void match_identifiers(TypedASTBlock* ast);
	void match_identifiers(TypedASTIfStatement* ast);
	void match_identifiers(TypedASTForStatement* ast);
	void match_identifiers(TypedASTFunctionLiteral* ast);
	void match_identifiers(TypedASTCallExpression* ast);
	void match_identifiers(TypedASTReturnStatement* ast);
	void match_identifiers(TypedASTBinaryExpression* ast);
	void match_identifiers(TypedASTIndexExpression* ast);
	void match_identifiers(TypedASTDeclarationList* ast);
};

void match_identifiers(TypedAST* ast) {
	FakeEnvironment env;

	// TODO: put this in a better place
	// HACK: this is an ugly hack. bear with me...
	TypedASTDeclaration dummy;

	env.declare_name("size", &dummy);
	env.declare_name("print", &dummy);
	env.declare_name("array_append", &dummy);
	env.declare_name("array_extend", &dummy);
	env.declare_name("array_join", &dummy);
	
	return env.match_identifiers(ast);
}

FakeEnvironment::FakeEnvironment() {
	m_scopes.push_back({ false });
}

TypedASTDeclaration* FakeEnvironment::access_name(std::string const& name) {
	auto scan_scope
	    = [](Scope& scope, std::string const& name) -> TypedASTDeclaration* {
		auto it = scope.m_vars.find(name);
		if (it != scope.m_vars.end())
			return it->second;
		return nullptr;
	};

	// iterate until we are no longer nested, excluding the global scope
	for (int i = m_scopes.size() - 1; i > 0; --i) {
		auto ptr = scan_scope(m_scopes[i], name);
		if (ptr)
			return ptr;

		// break if we are not nested
		if (!m_scopes[i].m_nested)
			break;
	}

	// now scan the global scope
	auto ptr = scan_scope(m_scopes[0], name);
	if (ptr)
		return ptr;

	return nullptr;
}

void FakeEnvironment::declare_name(std::string const& name, TypedASTDeclaration* decl) {
	auto& scope = m_scopes.back();
	scope.m_vars[name] = decl;
}

void FakeEnvironment::new_scope() {
	m_scopes.push_back({ false });
}

void FakeEnvironment::new_nested_scope() {
	m_scopes.push_back({ true });
}

void FakeEnvironment::end_scope() {
	m_scopes.pop_back();
}

void FakeEnvironment::match_identifiers(TypedASTDeclaration* ast) {
	declare_name(ast->identifier_text(), ast);
	if (ast->m_value) {
		match_identifiers(ast->m_value.get());
	}
}

void FakeEnvironment::match_identifiers(TypedASTIdentifier* ast) {
	TypedASTDeclaration* declaration = access_name(ast->text());
	assert(declaration);
	ast->m_declaration = declaration;
}

void FakeEnvironment::match_identifiers(TypedASTBlock* ast) {
	new_nested_scope();
	for (auto& child : ast->m_body)
		match_identifiers(child.get());
	end_scope();
}

void FakeEnvironment::match_identifiers(TypedASTIfStatement* ast) {
	match_identifiers(ast->m_condition.get());
	match_identifiers(ast->m_body.get());
}

void FakeEnvironment::match_identifiers(TypedASTCallExpression* ast) {
	match_identifiers(ast->m_callee.get());
	for (auto& arg : ast->m_args)
		match_identifiers(arg.get());
}

void FakeEnvironment::match_identifiers(TypedASTFunctionLiteral* ast) {
	// TODO: captures
	new_nested_scope();
	// declare arguments
	for (auto& decl : ast->m_args)
		match_identifiers(decl.get());
	// scan body
	assert(ast->m_body->type() == ast_type::Block);
	auto body = static_cast<TypedASTBlock*>(ast->m_body.get());
	for (auto& child : body->m_body)
		match_identifiers(child.get());
	end_scope();
}

void FakeEnvironment::match_identifiers(TypedASTForStatement* ast) {
	new_nested_scope();
	match_identifiers(ast->m_declaration.get());
	match_identifiers(ast->m_condition.get());
	match_identifiers(ast->m_action.get());
	match_identifiers(ast->m_body.get());
	end_scope();
}

void FakeEnvironment::match_identifiers(TypedASTReturnStatement* ast) {
	match_identifiers(ast->m_value.get());
}

void FakeEnvironment::match_identifiers(TypedASTBinaryExpression* ast) {
	match_identifiers(ast->m_lhs.get());
	match_identifiers(ast->m_rhs.get());
}

void FakeEnvironment::match_identifiers(TypedASTIndexExpression* ast) {
	match_identifiers(ast->m_callee.get());
	match_identifiers(ast->m_index.get());
}

void FakeEnvironment::match_identifiers(TypedASTDeclarationList* ast) {
	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedASTDeclaration*>(decl.get());
		declare_name(d->identifier_text(), d);
	}

	for (auto& decl : ast->m_declarations) {
		auto d = static_cast<TypedASTDeclaration*>(decl.get());
		if (d->m_value)
			match_identifiers(d->m_value.get());
	}
}

void FakeEnvironment::match_identifiers(TypedAST* ast) {
	// TODO: Compound literals
	switch (ast->type()) {
	case ast_type::Declaration:
		return match_identifiers(static_cast<TypedASTDeclaration*>(ast));
	case ast_type::Identifier:
		return match_identifiers(static_cast<TypedASTIdentifier*>(ast));
	case ast_type::Block:
		return match_identifiers(static_cast<TypedASTBlock*>(ast));
	case ast_type::ForStatement:
		return match_identifiers(static_cast<TypedASTForStatement*>(ast));
	case ast_type::IfStatement:
		return match_identifiers(static_cast<TypedASTIfStatement*>(ast));
	case ast_type::FunctionLiteral:
		return match_identifiers(static_cast<TypedASTFunctionLiteral*>(ast));
	case ast_type::CallExpression:
		return match_identifiers(static_cast<TypedASTCallExpression*>(ast));
	case ast_type::ReturnStatement:
		return match_identifiers(static_cast<TypedASTReturnStatement*>(ast));
	case ast_type::BinaryExpression:
		return match_identifiers(static_cast<TypedASTBinaryExpression*>(ast));
	case ast_type::IndexExpression:
		return match_identifiers(static_cast<TypedASTIndexExpression*>(ast));
	case ast_type::DeclarationList:
		return match_identifiers(static_cast<TypedASTDeclarationList*>(ast));
	case ast_type::StringLiteral:
	case ast_type::NumberLiteral:
	case ast_type::NullLiteral:
	case ast_type::BooleanLiteral:
		return;
	}
}

} // namespace TypeChecker
