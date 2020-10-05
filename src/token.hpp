#pragma once

#include <string>

#include "token_tag.hpp"
#include "interned_string.hpp"

struct Token {
	/* internal representation of token */
	TokenTag m_type;
	/* source code representation of token */
	InternedString m_text;

	/* beggining of token in source */
	int m_line0, m_col0;
	/* end of token in source */
	int m_line1, m_col1;
};
