#pragma once

#include "source_location.hpp"
#include "token_tag.hpp"
#include "utils/interned_string.hpp"

struct Token {
	/* internal representation of token */
	TokenTag m_type;
	/* source code representation of token */
	InternedString m_text;

	SourceRange m_source_location;
};
