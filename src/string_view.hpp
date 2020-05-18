#pragma once

#include <iosfwd>

struct string_view {
	char const* m_data;
	size_t m_size;

	string_view(char const*);
	string_view(const std::string&);

	int size() { return m_size; }
	char const* cbegin() const { return m_data; }
	char const* cend() const { return m_data + m_size; }
	char const* begin() const { return cbegin(); }
	char const* end() const { return cend(); }
};

std::ostream& operator << (std::ostream&, string_view const&);
