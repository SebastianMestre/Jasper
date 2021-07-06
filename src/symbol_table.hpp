#pragma once

#include <unordered_map>
#include <vector>

#include "./utils/interned_string.hpp"

namespace AST {
struct Declaration;
}

namespace Frontend {

struct SymbolTable {
	SymbolTable();

	AST::Declaration* access(InternedString const&);
	void declare(AST::Declaration*);

	void new_nested_scope();
	void end_scope();
private:
	using SymbolMap = std::unordered_map<InternedString, AST::Declaration*>;

	SymbolMap& latest_shadowed_scope();
	SymbolMap m_bindings;
	std::vector<SymbolMap> m_shadowed_scopes;
};

} // namespace Frontend
