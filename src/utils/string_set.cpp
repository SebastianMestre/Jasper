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
	m_data.resize(8);
	memset(m_data.data(), 0, sizeof(m_data[0]) * m_data.size());
}

std::pair<std::string const*, bool> StringSet::insert(std::string&& s) {
	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(s.data()), s.size());
	auto pos = scan(s.data(), s.size(), hash_bits);

	if (pos.found)
		return {m_data[pos.stop_index].value, false};

	m_size += 1;
	put(pos.free_index, std::move(s), hash_bits);

	return {m_data[pos.free_index].value, true};
}

std::pair<std::string const*, bool> StringSet::insert(char const* data, size_t length) {
	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(data), length);
	auto pos = scan(data, length, hash_bits);
	if (pos.found)
		return {m_data[pos.stop_index].value, false};

	m_size += 1;
	put(pos.free_index, std::string {data, length}, hash_bits);

	return {m_data[pos.free_index].value, true};
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

void StringSet::rehash(size_t new_size) {
	if (new_size < m_data.size())
		return;

	std::vector<HashField> old_data = std::move(m_data);

	m_data.clear();
	m_data.resize(new_size);
	memset(m_data.data(), 0, sizeof(m_data[0]) * m_data.size());

	m_size = 0;
	for (auto& slot : old_data) {
		if (slot.status != HashField::Occupied)
			continue;
		auto pos = scan(slot.value->data(), slot.value->size(), slot.hash_bits);
		assert(!pos.found);
		m_data[pos.free_index].value = slot.value;
		m_data[pos.free_index].status = HashField::Occupied;
		m_data[pos.free_index].hash_bits = slot.hash_bits;
		m_size += 1;
	}
}

StringSet::ScanData StringSet::scan(
    char const* data, size_t length, uint64_t hash_bits) const {
	assert(m_data.size() > m_size);

	const int capacity_mask = m_data.size() - 1;
	int position = hash_bits & capacity_mask;
	int free_position = -1;

	while (true) {
		if (m_data[position].status == HashField::Occupied) {
			if (m_data[position].hash_bits == hash_bits &&
			    length == m_data[position].value->size() &&
			    memcmp(data, m_data[position].value->data(), length) == 0)
				return {free_position, position, true};
		} else {
			if (free_position == -1)
				free_position = position;
			if (m_data[position].status == HashField::Empty)
				return {free_position, position, false};
		}

		position += 1;
		position &= capacity_mask;
	}
}

void StringSet::put(int position, std::string&& str, uint64_t hash_bits) {
	assert(m_data[position].status != HashField::Occupied);
	assert((hash_bits >> 62) == 0);

	m_storage.push_back(std::move(str));
	m_data[position].value = &m_storage.back();
	m_data[position].status = HashField::Occupied;
	m_data[position].hash_bits = hash_bits;
}
