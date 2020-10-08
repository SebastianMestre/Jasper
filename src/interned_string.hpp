#pragma once

#include <iosfwd>
#include <string>
#include <unordered_set>

struct StringSet;

struct InternedString {
	std::string const* m_data {nullptr};

	InternedString() = default;
	InternedString(InternedString const& other);
	InternedString(char const* other);
	InternedString(char const* other, size_t length);
	explicit InternedString(std::string const& other);
	explicit InternedString(std::string&& other);

	bool operator==(InternedString const& other) const {
		return m_data == other.m_data;
	}

	std::string const& str() const;

	static StringSet& database();
};

// Specialize std::hash to implement hashing for this type
template<> struct std::hash<InternedString> {
	std::size_t operator()(InternedString const& str) const noexcept {
		auto hash_bits = std::hash<std::string const*>{}(str.m_data);
		return (hash_bits >> 4) | (hash_bits << 60);
	};
};

std::ostream& operator<<(std::ostream&, InternedString const&);
