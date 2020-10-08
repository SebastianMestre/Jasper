#pragma once

#include <vector>

// Decently fast flat hash table
template<typename Key, typename Value>
struct FlatMap {

	// for every slot in the table, we also have a metadata byte.
	// if the first metadata bit is on:
	//   that slot is full, and the next 7 bits are hash bits,
	//   which we can use to save some checks
	// otherwise:
	//   the second bit is a tombstone (1) vs empty (0) flag
	//   and all the other bits are 0
	// 
	// thus, if a slot is empty, the metadata is 0x0.
	// and, if a slot is a tombstone, the metadata is 0x2.
	// and, if a slot is occupied, the metadata is an odd number.

	struct Magic {
		static constexpr uint8_t IsFullFlag = 0x1;
		static constexpr uint8_t IsTombstoneFlag = 0x2;

		static constexpr uint8_t Empty = 0x0;
		static constexpr uint8_t Tombstone = 0x2;
	};

	std::vector<uint8_t> m_metadata;
	std::vector<std::pair<const Key,Value>> m_slots;

	struct ScanResult {
		bool found;
		int end_idx;
		int free_idx;
	};

	ScanResult scan (Key const& key, uint64_t hash_bits) {
		uint64_t hi_hash_bits = hash_bits >> 7; // high 57 bits
		uint8_t  lo_hash_bits = hash_bits & ((1 << 7)-1); // low 7 bits

		uint8_t comparator = (lo_hash_bits << 1) | 1;
		int pos = hi_hash_bits % m_slots.size();
		int free_idx = -1;
		while (true) {
			if (m_metadata[pos] == comparator && m_slots[pos].first == key)
				return {true, pos, free_idx};

			if (m_metadata[pos] == Magic::Empty) {
				if (free_idx == -1)
					free_idx = pos;
				return {false, pos, free_idx};
			}

			if (free_idx == -1 && m_metadata[pos] == Magic::Tombstone)
				free_idx = pos;

			pos += 1;
			if (pos == m_slots.size())
				pos = 0;
		}
	}
};
