#include "automatic_block_allocator.hpp"

AutomaticBlockAllocator::AutomaticBlockAllocator(
    int bytes_per_object, int target_bytes_per_block)
    : m_allocator {
          bytes_per_slot_from_bytes_per_object(bytes_per_object),
          target_bytes_per_block} {}

// int AutomaticBlockAllocator::bytes_per_slot_from_bytes_per_object(int bytes_per_object) 
