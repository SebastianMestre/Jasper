#include "string_set.hpp"

#include "string_view.hpp"

#include <cassert>
#include <cstring>

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
}

void StringSet::insert(std::string const& s) {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(s.data()), s.size());
	auto pos = insertion_spot(s.data(), s.size(), hash_bits);

	if (pos.second)
		return;

	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

	m_size += 1;
	put(pos.first, std::string {s}, hash_bits);
}

void StringSet::insert(std::string&& s) {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(s.data()), s.size());
	auto pos = insertion_spot(s.data(), s.size(), hash_bits);

	if (pos.second)
		return;

	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

	m_size += 1;
	put(pos.first, std::move(s), hash_bits);
}

void StringSet::insert(char const* data, size_t length) {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(data), length);
	auto pos = insertion_spot(data, length, hash_bits);

	if (pos.second)
		return;

	if (m_size * 2 >= m_data.size())
		rehash(m_data.size() * 2);

	m_size += 1;
	put(pos.first, std::string {data, length}, hash_bits);
}

void StringSet::insert(char const* data) {
	insert(data, strlen(data));
}

bool StringSet::includes(char const* data, size_t length) const {
	uint64_t hash_bits = compute_effective_hash(
	    reinterpret_cast<unsigned char const*>(data), length);
	auto pos = find_spot(data, length, hash_bits);

	return pos.second;
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

	std::vector<std::pair<HashField, std::string>> old_data = std::move(m_data);

	m_data.resize(0);
	m_data.resize(new_size);
	for (auto& slot : old_data){
		if(slot.first.status == HashField::Occupied){
			uint64_t hash_bits = slot.first.hash_bits;
			auto pos = insertion_spot(
			    slot.second.data(),
			    slot.second.size(),
			    hash_bits);
			put(pos.first, std::move(slot.second), hash_bits);
		}
	}
}

std::pair<int, bool> StringSet::insertion_spot(char const* data, size_t length, uint64_t hash_bits) const {
	assert(m_data.size() > m_size);

	int position = hash_bits % m_data.size();
	bool found = false;
	while (m_data[position].first.status == HashField::Occupied) {

		if (length == m_data[position].second.size() &&
		    memcmp(data, m_data[position].second.data(), length) == 0) {
			found = true;
			break;
		}

		position += 1;
		if (position == static_cast<int>(m_data.size()))
			position = 0;
	}

	return {position, found};
}

std::pair<int, bool> StringSet::find_spot(char const* data, size_t length, uint64_t hash_bits) const {
	assert(m_data.size() > m_size);

	int position = hash_bits % m_data.size();
	bool found = false;
	while (m_data[position].first.status != HashField::Empty) {

		if (length == m_data[position].second.size() &&
		    memcmp(data, m_data[position].second.data(), length) == 0) {
			found = true;
			break;
		}

		position += 1;
		if (position == static_cast<int>(m_data.size()))
			position = 0;
	}

	return {position, found};
}

void StringSet::put(int position, std::string&& str, uint64_t hash_bits){
	assert(m_data[position].first.status != HashField::Occupied);

	m_data[position].first.status = HashField::Occupied;
	m_data[position].first.hash_bits = hash_bits;
	m_data[position].second = std::move(str);
}
