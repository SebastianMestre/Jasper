#include "automatic_block_allocator.hpp"

#include <cstdio>

AutomaticBlockAllocator::AutomaticBlockAllocator(
    int bytes_per_object, int target_bytes_per_block)
    : m_allocator {
          bytes_per_slot_from_bytes_per_object(bytes_per_object),
          target_bytes_per_block} {}

AutomaticBlockAllocator::~AutomaticBlockAllocator() {
	auto clear_block = [&](uint8_t* block, int bytes) {
		uint8_t* end = block + bytes;
		block += 8; // skip next block pointer
		for (; block < end; block += m_allocator.m_bytes_per_slot) {
			Destructor* dtor;
			memcpy(&dtor, block, 8);
			dtor(static_cast<void*>(block + 8));
		}
	};

	int bytes_to_clear = m_allocator.m_bytes_used_in_block;
	for (uint8_t* it = m_allocator.m_data; it; memcpy(&it, it, 8)) {
		clear_block(it, bytes_to_clear);
		bytes_to_clear = m_allocator.m_bytes_per_block;
	}
}

// int AutomaticBlockAllocator::bytes_per_slot_from_bytes_per_object(int bytes_per_object)
