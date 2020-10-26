#include "automatic_block_allocator.hpp"

AutomaticBlockAllocator::AutomaticBlockAllocator(
    int bytes_per_object, int target_bytes_per_block)
    : m_allocator {
          bytes_per_slot_from_bytes_per_object(bytes_per_object),
          target_bytes_per_block} {}

int AutomaticBlockAllocator::bytes_per_slot_from_bytes_per_object(int bytes_per_object) {
	// round up to multiple of 8
	int rounded_bytes_per_object = (bytes_per_object - 1) + 8 - ((bytes_per_object - 1) % 8);
	int slot_size = rounded_bytes_per_object + bytes_per_slot_header;
	return slot_size;
}
