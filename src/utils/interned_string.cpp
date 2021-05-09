#include "interned_string.hpp"

#include "string_set.hpp"

#include <cassert>
#include <cstring>

StringSet& InternedString::database() {
	static StringSet values;
	return values;
}

InternedString::InternedString(InternedString const& other)
    : m_data {other.m_data} {}

InternedString::InternedString(char const* other, size_t length) {
	auto insertion_result = database().insert(other, length);
	m_data = &(*insertion_result.first);
}

InternedString::InternedString(char const* other) {
	auto insertion_result = database().insert(other);
	m_data = &(*insertion_result.first);
}

InternedString::InternedString(string_view other)
    : InternedString {other.data(), other.size()} {}

InternedString::InternedString(std::string const& other) {
	auto insertion_result = database().insert(other);
	m_data = &(*insertion_result.first);
}

InternedString::InternedString(std::string&& other) {
	auto insertion_result = database().insert(std::move(other));
	m_data = &(*insertion_result.first);
}

std::string const& InternedString::str() const {
	assert(m_data);
	return *m_data;
}

bool operator== (string_view lhs, InternedString const& rhs) {
	if (rhs.is_null()) return false;
	return lhs.size() == rhs.size() && memcmp(rhs.str().data(), lhs.data(), lhs.size()) == 0;
}

bool operator== (InternedString const& lhs, string_view rhs) {
	if (lhs.is_null()) return false;
	return lhs.size() == rhs.size() && memcmp(lhs.str().data(), rhs.data(), rhs.size()) == 0;
}

std::ostream& operator<<(std::ostream& o, InternedString const& is) {
	return o << is.str();
}
