#include <iosfwd>
#include <string>
#include <unordered_set>

struct StringSet;

struct InternedString {
	std::string const* m_data {nullptr};

	InternedString(InternedString const& other);
	InternedString(char const* other);
	explicit InternedString(std::string const& other);
	explicit InternedString(std::string&& other);

	bool operator==(InternedString const& other) const;

	std::string const& str() const;

	static StringSet& database();
};

std::ostream& operator<<(std::ostream&, InternedString const&);
