#include "string_set.hpp"

#include "string_view.hpp"

#include <cassert>
#include <cstring>

#include <iostream>

// compute a 64-bit hash using a simple horner hashing scheme
static uint64_t compute_hash(unsigned char const* data, size_t length) {
	constexpr uint64_t base = 263;
	// TODO find a larger prime (less than 2**64/263)
	constexpr uint64_t mod = 1000000005721;

	uint64_t result = 0;
	while (length--) {
		result *= base;
		result += data[length];
		result %= mod;
	}

	return result;
}

// compute a 62-bit hash using a simple horner hashing scheme
static uint64_t compute_effective_hash(unsigned char const* data, size_t length) {
	uint64_t hash_bits = compute_hash(data, length);
	// roll the two highest bits back to the low bits
	// NOTE: due to how we implement compute_hash, the two highest bits are
	// always off. Still, doing this is pretty much free, and it makes this
	// function more future-proof.
	return (hash_bits >> 62) ^ hash_bits;
}

// ==== ==== ==== ====

StringSet::StringSet(){
	m_data.resize(8);
	memset(m_data.data(), 0, sizeof(m_data[0]) * m_data.size());
}

std::pair<std::string const*, bool> StringSet::insert(std::string&& s) {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(s.data()), s.size());
	auto pos = scan(s.data(), s.size(), hash_bits);

	if (pos.found)
		return {m_data[pos.stop_index].value, false};

	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

	m_size += 1;
	put(pos.free_index, std::move(s), hash_bits);

	return {m_data[pos.free_index].value, true};
}

std::pair<std::string const*, bool> StringSet::insert(char const* data, size_t length) {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(data), length);
	auto pos = scan(data, length, hash_bits);

	if (pos.found)
		return {m_data[pos.stop_index].value, false};

	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

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

	m_data.resize(0);
	m_data.resize(new_size);
	memset(m_data.data(), 0, sizeof(m_data[0]) * m_data.size());

	m_size = 0;
	for (auto& slot : old_data){
		if(slot.status == HashField::Occupied){
			auto pos = scan(slot.value->data(), slot.value->size(), slot.hash_bits);
			std::cerr << "INDEX " << pos.free_index << " STRING \"" << *slot.value << "\"\n";
			m_data[pos.free_index].value = slot.value;
			m_data[pos.free_index].status = HashField::Occupied;
			m_data[pos.free_index].hash_bits = slot.hash_bits;
			m_size += 1;
		}
	}
}


StringSet::ScanData StringSet::scan(char const* data, size_t length, uint64_t hash_bits) const {
	assert(m_data.size() > m_size);

	int position = hash_bits % m_data.size();
	int free_position = -1;
	bool found = false;

	while (m_data[position].status != HashField::Empty) {

		if (m_data[position].status == HashField::Tombstone) {
			if (free_position == -1)
				free_position = position;
		} else if (m_data[position].status == HashField::Occupied) {
			if (length == m_data[position].value->size() &&
			    memcmp(data, m_data[position].value->data(), length) == 0) {
				found = true;
				break;
			}
		}

		position += 1;
		if (position == static_cast<int>(m_data.size()))
			position = 0;
	}

	if (m_data[position].status == HashField::Empty) {
		if (free_position == -1)
			free_position = position;
	}

	return {free_position, position, found};
}

void StringSet::put(int position, std::string&& str, uint64_t hash_bits){
	std::cerr << "PUTTING AT " << position << " -- SIZE IS " << m_data.size() << "\n";

	assert(m_data[position].status != HashField::Occupied);

	m_storage.push_back(std::move(str));
	m_data[position].value = &m_storage.back();
	m_data[position].status = HashField::Occupied;
	m_data[position].hash_bits = hash_bits;
}
