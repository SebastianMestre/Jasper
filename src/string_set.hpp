#include <vector>
#include <string>
#include <utility>

#include <cstdint>

struct string_view;
struct InternedString;

// a flat linear hashing table
struct StringSet {
	struct HashField {
		// possible status values:
		static constexpr uint64_t Empty = 0;
		static constexpr uint64_t Occupied = 1;
		static constexpr uint64_t Tombstone = 2;

		uint64_t status : 2;
		uint64_t hash_bits : 62;
	};

	std::vector<std::pair<HashField, std::string>> m_data;
	size_t m_size;

	StringSet();

	void insert(std::string const&);
	void insert(std::string&&);
	// void insert(InternedString const&); // TODO
	// void insert(string_view const&); // TODO
	void insert(char const*, size_t);
	void insert(char const*);

	bool includes(std::string const&) const;
	// bool includes(InternedString const&) const;
	// bool includes(string_view const&) const;
	bool includes(char const*, size_t) const;
	bool includes(char const*) const;

  private:
	std::pair<int, bool> insertion_spot(char const*, size_t, uint64_t) const;
	std::pair<int, bool> find_spot(char const*, size_t, uint64_t) const;
	void put(int, std::string&&, uint64_t);
	void rehash(size_t);
};
