#include "symbol_table.hpp"

#include "ast.hpp"
#include "./log/log.hpp"

#include <cassert>

namespace Frontend {

SymbolTable::SymbolTable() {
	new_nested_scope();
}

SymbolTable::SymbolMap& SymbolTable::latest_shadowed_scope() {
	return m_shadowed_scopes.back();
}

void SymbolTable::declare(AST::Declaration* decl) {
	// @speed: to use SymbolMap::insert instead of SymbolTable::access
	auto old_decl = access(decl->identifier_text());

	auto insert_result =
	    latest_shadowed_scope().insert({decl->identifier_text(), old_decl});
	if (!insert_result.second)
		Log::fatal() << "Redeclaration of '" << decl->identifier_text() << "'";

	m_available_vars[decl->identifier_text()] = decl;
}

AST::Declaration* SymbolTable::access(InternedString const& name) {
	auto it = m_available_vars.find(name);
	if (it == m_available_vars.end())
		return nullptr;
	return it->second;
}

void SymbolTable::new_nested_scope() {
	m_shadowed_scopes.push_back({});
}

void SymbolTable::end_scope() {
	auto& prev_scope = latest_shadowed_scope();

	for (auto& kv : prev_scope) {
		if (kv.second)
			m_available_vars[kv.first] = kv.second;
		else
			m_available_vars.erase(kv.first);
	}

	m_shadowed_scopes.pop_back();
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
