#include "block_allocator.hpp"

#include <algorithm>

#include <cassert>
#include <cstdio>
#include <cstring>

BlockAllocator::BlockAllocator(int bytes_per_slot, int target_bytes_per_block)
    : m_bytes_per_slot {bytes_per_slot}
    , m_data {nullptr} {

	// we assume no alignment requirements
	constexpr int bytes_per_block_header = 8;
	int const target_bytes_per_block_body = target_bytes_per_block - bytes_per_block_header;

	// ensure we allocate at least 1 object
	// otherwise, don't go over the target block size
	int const slot_count = std::max(1, target_bytes_per_block_body / bytes_per_slot);

	m_bytes_per_block = bytes_per_block_header + slot_count * bytes_per_slot;

	new_block();
}

BlockAllocator::~BlockAllocator() {
	while (m_data) {
		auto to_delete = m_data;
		memcpy(&m_data, m_data, 8);
		delete[] to_delete;
	}
}

void BlockAllocator::new_block() {
	// fprintf(stderr, "allocating new block\n");
	auto const old_data = m_data;
	m_data = new uint8_t[m_bytes_per_block];
	memcpy(m_data, &old_data, 8); // assume a pointer is 8 bytes
	m_bytes_used_in_block = 8;
}

// bool BlockAllocator::block_is_full() const { }

// uint8_t* BlockAllocator::allocate(int bytes) { }
