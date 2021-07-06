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

	auto insert_result =
	    latest_shadowed_scope().insert({decl->identifier_text(), old_decl});
	if (!insert_result.second)
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





} // namespace Frontend
