#include <cassert>
#include <iostream>

#include "./log/log.hpp"
#include "cst.hpp"
#include "ast.hpp"

namespace AST {

InternedString const& Declaration::identifier_text() const {
	if (m_identifier.is_null()) {
		if (!m_cst)
			Log::fatal() << "No identifier string or fallback on declaration";

		auto cst = static_cast<CST::Declaration*>(m_cst);

		auto const& found_identifier = cst->identifier_virtual();

		Log::warning() << "No identifier on declaration, using token data as fallback: '" << found_identifier << "'";

		return found_identifier;
	}

	return m_identifier;
}

Token const* Identifier::token() const {
	return static_cast<CST::Identifier*>(m_cst)->m_token;
}

} // namespace AST
