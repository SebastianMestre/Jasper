#pragma once

#include <type_traits>
#include <utility>
#include <vector>

#include <cstdint>

// Decently fast flat hash table
// If rehashing occurs, iterators are invalidated
// If rehashing occurs, references are invalidated
template<typename Key, typename Value>
struct FlatMap {
	using value_type = std::pair<Key, Value>;
	using const_iterator = value_type const*;
	using iterator = const_iterator;

	FlatMap() {
		constexpr int initial_size = 16;
		m_metadata.resize(initial_size, 0);
		m_slots.resize(initial_size);
	}

	std::pair<iterator, bool> insert(const value_type& value) {
		uint64_t hash_bits = m_hash(value.first);
		auto scan_result = scan(value.first, hash_bits);
		if (scan_result.found)
			return {&m_slots[scan_result.end_idx], false};
		put(scan_result.free_idx, value_type {value}, hash_bits);
		return {&m_slots[scan_result.free_idx], true};
	}

	std::pair<iterator, bool> insert(value_type&& value) {
		uint64_t hash_bits = m_hash(value.first);
		auto scan_result = scan(value.first, hash_bits);
		if (scan_result.found)
			return {&m_slots[scan_result.end_idx], false};
		put(scan_result.free_idx, std::move(value), hash_bits);
		return {&m_slots[scan_result.free_idx], true};
	}

	template <typename P>
	typename std::enable_if<
		std::is_constructible<value_type, P&&>::value,
		std::pair<iterator, bool>>::type insert(P&& value) {
		return insert(value_type {std::move(value)});
	}

private:

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
	std::vector<value_type> m_slots;
	std::hash<Key> m_hash;

	struct ScanResult {
		bool found;
		int end_idx;
		int free_idx;
	};

	ScanResult scan (Key const& key, uint64_t hash_bits) {
		uint64_t hi_hash_bits = hash_bits >> 7; // high 57 bits
		uint8_t  lo_hash_bits = hash_bits & ((1 << 7)-1); // low 7 bits

		uint8_t comparator = (lo_hash_bits << 1) | 1;
		size_t pos = hi_hash_bits % m_slots.size();
		int free_idx = -1;
		while (true) {
			if (m_metadata[pos] == comparator && m_slots[pos].first == key)
				return {true, (int)pos, free_idx};

			if (m_metadata[pos] == Magic::Empty) {
				if (free_idx == -1)
					free_idx = pos;
				return {false, (int)pos, free_idx};
			}

			if (free_idx == -1 && m_metadata[pos] == Magic::Tombstone)
				free_idx = pos;

			pos += 1;
			if (pos == m_slots.size())
				pos = 0;
		}
	}

	void put(int position, value_type&& value, uint64_t hash_bits){
		uint8_t  lo_hash_bits = hash_bits & ((1 << 7)-1); // low 7 bits
		uint8_t comparator = (lo_hash_bits << 1) | 1;

		m_metadata[position] = comparator;
		m_slots[position] = std::move(value);
	}

};
