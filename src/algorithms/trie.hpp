#include <array>
#include <vector>

#include <cstdint>

#include "../utils/string_view.hpp"
#include "../utils/interned_string.hpp"

static constexpr uint8_t nil = ~uint8_t(0);

struct Trie {
	struct Entry {
		InternedString text;
		int data;
	};

	struct Node {
		Node();
		Node(Node&&) = default;
		Node(Node const&) = default;

		uint8_t entry {nil};
		std::array<uint8_t, 255> children;
	};

	std::vector<Node> nodes;
	std::vector<Entry> entries;

	Trie();

	uint8_t go(uint8_t n, uint8_t c);
	void insert(Entry);
	bool has(string_view);
	Entry longest_prefix_of(string_view);
};

Trie build_trie (std::vector<Trie::Entry> strings);
