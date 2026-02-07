#include "token.hpp"

std::ostream& operator<<(std::ostream& o, Token const* token) {
	return o << token->m_text;
}