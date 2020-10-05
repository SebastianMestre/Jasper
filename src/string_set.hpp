#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "chunked_array.hpp"

#include <cstdint>

struct string_view;
struct InternedString;

// a flat linear hashing table
struct StringSet {
	struct ScanData {
		int free_index;
		int stop_index;
		bool found;
	};

	struct HashField {
		// possible status values:
		static constexpr uint64_t Empty = 0;
		static constexpr uint64_t Occupied = 1;
		static constexpr uint64_t Tombstone = 2;

		std::string* value;
		uint64_t status : 2;
		uint64_t hash_bits : 62;
	};

	ChunkedArray<std::string> m_storage;
	std::vector<HashField> m_data;
	size_t m_size {0};

	StringSet();

	std::pair<std::string const*, bool> insert(std::string const&);
	std::pair<std::string const*, bool> insert(std::string&&);
	// void insert(InternedString const&); // TODO
	// void insert(string_view const&); // TODO
	std::pair<std::string const*, bool> insert(char const*, size_t);
	std::pair<std::string const*, bool> insert(char const*);

	bool includes(std::string const&) const;
	// bool includes(InternedString const&) const; // TODO
	// bool includes(string_view const&) const; // TODO
	bool includes(char const*, size_t) const;
	bool includes(char const*) const;

  private:
	ScanData scan(char const*, size_t, uint64_t) const;
	void put(int, std::string&&, uint64_t);
	void rehash(size_t);
};
