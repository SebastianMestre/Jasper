#pragma once

#include <string>

#include "token_type.hpp"

struct Token {
	/* internal representation of token */
	TokenTag m_type;
	/* source code representation of token */
	std::string m_text;

	/* beggining of token in source */
	int m_line0, m_col0;
	/* end of token in source */
	int m_line1, m_col1;
};
