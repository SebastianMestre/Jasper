#include "trie.hpp"
#include <iostream>
#include <cassert>

Trie::Node::Node() {
	children.fill(nil);
}

Trie::Trie() {
	nodes.push_back(Node {});
}

uint8_t Trie::go(uint8_t node, uint8_t uc) {
	return nodes[node].children[uc];
}

void Trie::insert(Entry entry) {
	int entry_idx = entries.size();
	entries.push_back(entry);

	int node = 0;
	for (char c : entry.text) {
		uint8_t uc = c;
		auto next_node = go(node, uc);
		if (next_node == nil) {
			next_node = nodes[node].children[uc] = nodes.size();
			nodes.push_back(Node {});
		}

		node = next_node;
	}

	assert(nodes[node].entry == nil);
	nodes[node].entry = entry_idx;
}

bool Trie::has(string_view str) {
	int node = 0;
	for (char c : str) {
		uint8_t uc = c;
		auto next_node = go(node, uc);
		if (next_node == nil)
			return false;

		node = next_node;
	}
	return nodes[node].entry != nil;
}

Trie::Entry Trie::longest_prefix_of(string_view haystack) {
	uint8_t entry_idx = nil;
	uint8_t node = 0;

	for (char c : haystack) {
		uint8_t uc = c;
		uint8_t next_node = go(node, uc);
		if (next_node == nil)
			break;

		node = next_node;

		if (nodes[node].entry != nil)
			entry_idx = nodes[node].entry;
	}

	if (entry_idx == nil)
		return {};
	return entries[entry_idx];
}

Trie build_trie(std::vector<Trie::Entry> entries) {
	Trie result;
	for (auto const& entry : entries)
		result.insert(entry);
	return result;
}
