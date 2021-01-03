#include <array>
#include <vector>

#include <cstdint>

#include "../utils/string_view.hpp"

static constexpr uint16_t nil = ~uint16_t(0);

struct Trie {
	struct Entry {
		string_view text;
		int data;
	};

	struct Node {
		Node();
		Node(Node&&) = default;
		Node(Node const&) = default;

		uint16_t entry {nil};
		std::array<uint16_t, 255> children;
	};

	std::vector<Node> nodes;
	std::vector<Entry> entries;

	Trie();

	uint16_t go(uint16_t n, uint8_t c);
	void insert(Entry);
	bool has(string_view);
	Entry longest_prefix_of(string_view);
};

Trie build_trie (std::vector<Trie::Entry> strings);
