#include "symbol_table.hpp"

#include "ast.hpp"
#include "./log/log.hpp"

#include <cassert>

namespace Frontend {

SymbolTable::Scope& SymbolTable::current_scope() {
	return m_scopes.empty() ? m_global_scope : m_scopes.back();
}

void SymbolTable::declare(AST::Declaration* decl) {
	auto insert_result = current_scope().m_vars.insert({decl->identifier_text(), decl});
	if (!insert_result.second)
		Log::fatal() << "Redeclaration of '" << decl->identifier_text() << "'";
}

AST::Declaration* SymbolTable::access(InternedString const& name) {
	auto scan_scope = [](Scope& scope, InternedString const& name) -> AST::Declaration* {
		auto it = scope.m_vars.find(name);
		if (it != scope.m_vars.end())
			return it->second;
		return nullptr;
	};

	// scan nested scopes from the inside out
	for (int i = m_scopes.size(); i--;) {
		auto ptr = scan_scope(m_scopes[i], name);
		if (ptr)
			return ptr;
		if (!m_scopes[i].m_nested)
			break;
	}

	// fall back to global scope lookup
	return scan_scope(m_global_scope, name);
}

void SymbolTable::new_scope() {
	m_scopes.push_back({false});
}

void SymbolTable::new_nested_scope() {
	m_scopes.push_back({true});
}

void SymbolTable::end_scope() {
	m_scopes.pop_back();
}

AST::SequenceExpression* SymbolTable::current_seq_expr() {
	return m_seq_expr_stack.empty() ? nullptr : m_seq_expr_stack.back();
}

void SymbolTable::enter_seq_expr(AST::SequenceExpression* seq_expr) {
	m_seq_expr_stack.push_back(seq_expr);
}

void SymbolTable::exit_seq_expr() {
	m_seq_expr_stack.pop_back();
}

AST::FunctionLiteral* SymbolTable::current_function() {
	return m_function_stack.empty() ? nullptr : m_function_stack.back();
}

void SymbolTable::enter_function(AST::FunctionLiteral* func) {
	m_function_stack.push_back(func);
}

void SymbolTable::exit_function() {
	m_function_stack.pop_back();
}

AST::Declaration* SymbolTable::current_top_level_declaration() {
	return m_current_decl;
}

void SymbolTable::enter_top_level_decl(AST::Declaration* decl) {
	assert(!m_current_decl);
	m_current_decl = decl;
}

void SymbolTable::exit_top_level_decl() {
	assert(m_current_decl);
	m_current_decl = nullptr;
}

} // namespace Frontend
