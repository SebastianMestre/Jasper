#pragma once

#include <vector>

#include "typechecker_types.hpp"

namespace TypeChecker {

// TODO: add monotypes and polytypes?
enum class MetaTypeTag {
	Var, // unification variable
	Meta, // a meta type, like "type function", "mono type" or "value"
};

struct MetaTypeHeader {
	MetaTypeTag tag;
	int equals;
	// NOTE(SMestre): i dislike the name 'equals' but let's
	// keep it consistent with the rest of the type system
	char const* debug_data;
};

struct MetaTypeSystem {
	std::vector<MetaTypeHeader> m_metatype_header;

	MetaTypeSystem() = default;

	void unify(MetaTypeId, MetaTypeId);
	MetaTypeId find(MetaTypeId);

	MetaTypeId new_var(char const* debug_data = nullptr);
	MetaTypeId new_meta(char const* debug_data = nullptr);
};

} // namespace TypeChecker
