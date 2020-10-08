#pragma once

#include <type_traits>
#include <utility>
#include <vector>

#include <cstdint>

// Decently fast flat hash table
// If rehashing occurs, iterators are invalidated
// If rehashing occurs, references are invalidated
// Good when comparing keys is cheap
template<typename Key, typename Value>
struct SimpleFlatMap {
	using value_type = std::pair<Key, Value>;
	using const_iterator = value_type const*;
	using iterator = const_iterator;

	SimpleFlatMap() {
		// MUST be power of two
		constexpr int initial_size = 64;
		m_slots.resize(initial_size);
		// 2 bits per element = one byte every 4 elements
		m_metadata.resize((initial_size+3)/4, 0);
	}

	std::pair<iterator, bool> insert(const value_type& value) {
		rehash_if_needed();
		uint64_t hash_bits = m_hash(value.first);
		auto scan_result = scan(value.first, hash_bits);
		if (scan_result.found)
			return {&m_slots[scan_result.end_idx], false};
		m_size += 1;
		put(scan_result.free_idx, value_type {value}, hash_bits);
		return {&m_slots[scan_result.free_idx], true};
	}

	std::pair<iterator, bool> insert(value_type&& value) {
		rehash_if_needed();
		uint64_t hash_bits = m_hash(value.first);
		auto scan_result = scan(value.first, hash_bits);
		if (scan_result.found)
			return {&m_slots[scan_result.end_idx], false};
		m_size += 1;
		put(scan_result.free_idx, std::move(value), hash_bits);
		return {&m_slots[scan_result.free_idx], true};
	}

	template <typename P>
	typename std::enable_if<
		std::is_constructible<value_type, P&&>::value,
		std::pair<iterator, bool>>::type insert(P&& value) {
		return insert(value_type {std::move(value)});
	}

	Value& operator[](Key const& k) {
		rehash_if_needed();
		uint64_t hash_bits = m_hash(k);
		auto scan_result = scan(k, hash_bits);
		if (scan_result.found)
			return m_slots[scan_result.end_idx].second;
		m_size += 1;
		put(scan_result.free_idx, value_type {k, {}}, hash_bits);
		return m_slots[scan_result.free_idx].second;
	}

	Value& operator[](Key&& k) {
		rehash_if_needed();
		uint64_t hash_bits = m_hash(k);
		auto scan_result = scan(k, hash_bits);
		if (scan_result.found)
			return m_slots[scan_result.end_idx].second;
		m_size += 1;
		put(scan_result.free_idx, value_type {std::move(k), {}}, hash_bits);
		return m_slots[scan_result.free_idx].second;
	}

	int count(Key const& k) const {
		uint64_t hash_bits = m_hash(k);
		auto scan_result = scan(k, hash_bits);
		return int(scan_result.found);
	}

	iterator find(Key const& k) const {
		uint64_t hash_bits = m_hash(k);
		auto scan_result = scan(k, hash_bits);
		if (scan_result.found)
			return &m_slots[scan_result.end_idx];
		return end();
	}

	iterator end() const {
		return m_slots.data() + m_slots.size();
	}

	iterator begin() const {
		return m_slots.data();
	}

private:

	// for every slot in the table, we also have two metadata bits.
	// if the first metadata bit is on:
	//   that slot is full, and the next bit is 0.
	// otherwise:
	//   the second bit is a tombstone (1) vs empty (0) flag
	// 
	// thus, if a slot is empty, the metadata is 0x0.
	// if a slot is occupied, the metadata is 0x1.
	// and, if a slot is a tombstone, the metadata is 0x2.

	struct Magic {
		static constexpr uint8_t IsFullFlag = 0x1;
		static constexpr uint8_t IsTombstoneFlag = 0x2;

		static constexpr uint8_t Empty = 0x0;
		static constexpr uint8_t Full = 0x1;
		static constexpr uint8_t Tombstone = 0x2;
	};

	std::vector<uint8_t> m_metadata;
	std::vector<value_type> m_slots;
	size_t m_size {0};
	std::hash<Key> m_hash;

	struct ScanResult {
		bool found;
		int end_idx;
		int free_idx;
	};

	ScanResult scan (Key const& key, uint64_t hash_bits) const {
		size_t pos = hash_bits & (m_slots.size() - 1);
		int free_idx = -1;
		while (true) {
			uint8_t metadata = (m_metadata[pos/4] >> ((pos%4)*2)) & 0x3;

			if (metadata == Magic::Full && m_slots[pos].first == key)
				return {true, (int)pos, free_idx};

			if (metadata == Magic::Empty) {
				if (free_idx == -1)
					free_idx = pos;
				return {false, (int)pos, free_idx};
			}

			if (free_idx == -1 && metadata == Magic::Tombstone)
				free_idx = pos;

			pos += 1;
			if (pos == m_slots.size())
				pos = 0;
		}
	}

	void put(int position, value_type&& value, uint64_t hash_bits){
		m_metadata[position/4] &= ~(0x3 << ((position%4)*2));
		m_metadata[position/4] |= Magic::Full << ((position%4)*2);
		m_slots[position] = std::move(value);
	}

	void rehash_if_needed() {
		if (m_slots.size() <= m_size * 2)
			rehash(m_slots.size() * 2);
	}

	void rehash(size_t new_size) {
		if (new_size < m_slots.size())
			return;

		std::vector<value_type> old_slots = std::move(m_slots);
		m_slots.clear();
		m_slots.resize(new_size);

		std::vector<uint8_t> old_metadata = std::move(m_metadata);
		m_metadata.assign(new_size, 0);

		for (size_t i {0}; i != old_slots.size(); ++i) {
			if (!(old_metadata[i] & Magic::IsFullFlag))
				continue;

			value_type& slot = old_slots[i];
			uint64_t hash_bits = m_hash(slot.first);
			uint64_t hi_hash_bits = hash_bits >> 7;
			size_t pos = hi_hash_bits & (m_slots.size() - 1);
			// we know there are no repeats, so we can do something simpler than `scan`
			while (true) {
				if (m_metadata[pos] == Magic::Empty)
					break;
				pos += 1;
				if (pos == m_slots.size())
					pos = 0;
			}

			put(pos, std::move(old_slots[i]), hash_bits);
		}
	}
};
