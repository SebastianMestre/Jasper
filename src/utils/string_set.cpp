#include "string_set.hpp"

#include <cassert>
#include <cstring>

// compute a 64-bit hash
static uint64_t compute_hash(unsigned char const* data, size_t length) {
	// djb2 (k=33) hash function
	uint64_t result = 5381;
	while (length--) {
		result *= 33;
		result ^= data[length];
	}
	return result;
}

// compute a 62-bit hash
static uint64_t compute_effective_hash(unsigned char const* data, size_t length) {
	uint64_t hash_bits = compute_hash(data, length);
	// roll the two highest bits back to the low bits
	return (hash_bits >> 62) ^ (hash_bits & ~(3ull << 62));
}

// ==== ==== ==== ====

StringSet::StringSet() {
	constexpr int initial_size = 8;
	m_slot.resize(initial_size);
	memset(m_slot.data(), 0, sizeof(m_slot[0]) * initial_size);
	m_value.resize(initial_size);
	memset(m_value.data(), 0, sizeof(m_value[0]) * initial_size);
}

std::pair<std::string const*, bool> StringSet::insert(std::string&& s) {
	if (m_size * 2 >= m_slot.size())
		rehash(m_slot.size() * 2);

	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(s.data()), s.size());
	auto pos = scan(s.data(), s.size(), hash_bits);

	if (pos.found)
		return {m_value[pos.stop_index], false};

	m_size += 1;
	put(pos.free_index, std::move(s), hash_bits);

	return {m_value[pos.free_index], true};
}

std::pair<std::string const*, bool> StringSet::insert(char const* data, size_t length) {
	if (m_size * 2 >= m_slot.size())
		rehash(m_slot.size() * 2);

	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(data), length);
	auto pos = scan(data, length, hash_bits);
	if (pos.found)
		return {m_value[pos.stop_index], false};

	m_size += 1;
	put(pos.free_index, std::string {data, length}, hash_bits);

	return {m_value[pos.free_index], true};
}

std::pair<std::string const*, bool> StringSet::insert(std::string const& s) {
	return insert(s.data(), s.size());
}

std::pair<std::string const*, bool> StringSet::insert(char const* data) {
	return insert(data, strlen(data));
}

bool StringSet::includes(char const* data, size_t length) const {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(data), length);
	auto pos = scan(data, length, hash_bits);

	return pos.found;
}

bool StringSet::includes(char const* data) const {
	return includes(data, strlen(data));
}

bool StringSet::includes(std::string const& str) const {
	return includes(str.data(), str.size());
}

// ==== ==== ==== ====

void StringSet::rehash(size_t const new_size) {
	if (new_size < m_slot.size())
		return;

	std::vector<HashField> old_slot = std::move(m_slot);
	m_slot.clear();
	m_slot.resize(new_size);
	memset(m_slot.data(), 0, sizeof(m_slot[0]) * new_size);

	std::vector<std::string*> old_value = std::move(m_value);
	m_value.clear();
	m_value.resize(new_size);
	memset(m_value.data(), 0, sizeof(m_value[0]) * new_size);

	m_size = 0;
	const int n = old_slot.size();
	for (int i = 0; i < n; ++i) {
		auto& slot = old_slot[i];

		if (slot.status != HashField::Occupied)
			continue;

		auto value = old_value[i];
		auto pos = scan(value->data(), value->size(), slot.hash_bits);
		assert(!pos.found);
		m_value[pos.free_index] = value;
		m_slot[pos.free_index].status = HashField::Occupied;
		m_slot[pos.free_index].hash_bits = slot.hash_bits;
		m_size += 1;
	}
}

StringSet::ScanData StringSet::scan(
    char const* data, size_t length, uint64_t hash_bits) const {
	assert(m_slot.size() > m_size);

	// ensure that m_slot.size() is a power of 2
	assert((m_slot.size() & (m_slot.size() - 1)) == 0);

	// m_slot.size() being a power of 2 lets us use bit
	// masking instead of modulo to wrap indices
	const int capacity_mask = m_slot.size() - 1;
	int position = hash_bits & capacity_mask;
	int free_position = -1;

	while (true) {
		if (m_slot[position].status == HashField::Occupied) {
			if (m_slot[position].hash_bits == hash_bits &&
			    length == m_value[position]->size() &&
			    memcmp(data, m_value[position]->data(), length) == 0)
				return {free_position, position, true};
		} else {
			if (free_position == -1)
				free_position = position;
			if (m_slot[position].status == HashField::Empty)
				return {free_position, position, false};
		}

		position += 1;
		position &= capacity_mask;
	}
}

void StringSet::put(int position, std::string&& str, uint64_t hash_bits) {
	assert(m_slot[position].status != HashField::Occupied);
	assert((hash_bits >> 62) == 0);

	m_storage.push_back(std::move(str));
	m_value[position] = &m_storage.back();
	m_slot[position].status = HashField::Occupied;
	m_slot[position].hash_bits = hash_bits;
}
