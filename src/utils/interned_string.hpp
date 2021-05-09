#pragma once

#include <iosfwd>
#include <string>

#include "string_view.hpp"

struct StringSet;

struct InternedString {
	using const_iterator = char const*;
	using iterator = const_iterator;

	std::string const* m_data {nullptr};

	InternedString() = default;
	InternedString(InternedString const& other);
	InternedString(char const* other);
	InternedString(char const* other, size_t length);
	explicit InternedString(string_view other);
	explicit InternedString(std::string const& other);
	explicit InternedString(std::string&& other);

	bool is_null() const {
		return !m_data;
	}

	size_t size() const {
		return is_null() ? 0 : m_data->size();
	}

	bool operator==(InternedString const& other) const {
		return m_data == other.m_data;
	}

	bool operator<(InternedString const& other) const {
		return m_data < other.m_data;
	}

	std::string const& str() const;

	static StringSet& database();

	const_iterator cbegin() const {
		return m_data->data();
	}

	const_iterator cend() const {
		return m_data->data() + m_data->size();
	}

	iterator begin() {
		return cbegin();
	}

	iterator end() {
		return cend();
	}
};

// Specialize std::hash to implement hashing for this type
template<> struct std::hash<InternedString> {
	std::size_t operator()(InternedString const& str) const noexcept {
		auto hash_bits = std::hash<std::string const*>{}(str.m_data);
		return (hash_bits >> 4) | (hash_bits << 60);
	};
};

bool operator== (string_view lhs, InternedString const& rhs);

bool operator== (InternedString const& lhs, string_view rhs);

std::ostream& operator<<(std::ostream&, InternedString const&);
