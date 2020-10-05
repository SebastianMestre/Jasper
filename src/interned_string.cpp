#include "interned_string.hpp"

#include <iostream>

#include "string_set.hpp"

#include <cassert>

StringSet& InternedString::database() {
	static StringSet values;
	return values;
}

InternedString::InternedString(InternedString const& other)
    : m_data {other.m_data} {}

InternedString::InternedString(char const* other)
    : InternedString(std::string(other)) {}

InternedString::InternedString(std::string const& other) {
	auto insertion_result = database().insert(other);
	m_data = &(*insertion_result.first);
}

InternedString::InternedString(std::string&& other) {
	auto insertion_result = database().insert(std::move(other));
	m_data = &(*insertion_result.first);
}

bool InternedString::operator==(InternedString const& other) const {
	return m_data == other.m_data;
}

std::string const& InternedString::str() const {
	assert(m_data);
	return *m_data;
}

std::ostream& operator<<(std::ostream& o, InternedString const& is) {
	return o << is.str();
}
