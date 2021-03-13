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
	auto old_decl = [&]() -> AST::Declaration* {
		auto insert_result = m_bindings.insert({decl->identifier_text(), decl});
		if (insert_result.second) // introduced a previously undeclared name
			return nullptr;
		return std::exchange(insert_result.first->second, decl);
	}();

	auto scope_insert_result =
	    latest_shadowed_scope().insert({decl->identifier_text(), old_decl});
	if (!scope_insert_result.second) // clobbers a name in the same scope
		Log::fatal() << "Redeclaration of '" << decl->identifier_text() << "'";
}

AST::Declaration* SymbolTable::access(InternedString const& name) {
	auto it = m_bindings.find(name);
	if (it == m_bindings.end())
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
			m_bindings[kv.first] = kv.second;
		else
			m_bindings.erase(kv.first);
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
