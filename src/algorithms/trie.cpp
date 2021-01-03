#include "trie.hpp"
#include <iostream>
#include <cassert>

Trie::Node::Node() {
	children.fill(nil);
}

Trie::Trie (){
	nodes.push_back(Node {});
}

uint16_t Trie::go(uint16_t n, uint8_t uc) {
	return nodes[n].children[uc];
}

void Trie::insert(Entry entry) {
	int entry_idx = entries.size();
	entries.push_back(entry);

	int n = 0;
	for (char c : entry.text) {
		uint8_t uc = c;
		auto nn = go(n, uc);
		if (nn == nil) {
			nn = nodes[n].children[uc] = nodes.size();
			nodes.push_back(Node {});
		}

		n = nn;
	}

	assert(nodes[n].entry == nil);
	nodes[n].entry = entry_idx;
}

bool Trie::has(string_view str) {
	int n = 0;
	for (char c : str) {
		uint8_t uc = c;
		auto nn = go(n, uc);
		if (nn == nil)
			return false;

		n = nn;
	}
	return nodes[n].entry != nil;
}

Trie::Entry Trie::longest_prefix_of(string_view haystack) {
	uint16_t entry_idx = nil;
	uint16_t n = 0;

	for (char c : haystack) {
		uint8_t uc = c;
		uint16_t nn = go(n, uc);
		if (nn == nil)
			break;

		n = nn;

		if (nodes[n].entry != nil)
			entry_idx = nodes[n].entry;
	}

	if (entry_idx == nil)
		return {};
	return entries[entry_idx];
}

void print_space(int n){
	for(int i = n; i--;)
		std::cout << "   ";
}

void Trie::print(int n, int d) const {
	for (int i = 0; i < 255; ++i) {
		if (nodes[n].children[i] != nil) {
			print_space(d);
			std::cout << char(i) << " (" << i << ")\n";

			print(nodes[n].children[i], d+1);
		}
	}
}

Trie build_trie (std::vector<Trie::Entry> entries) {
	Trie result;
	for (auto const& entry : entries)
		result.insert(entry);
	return result;
}

