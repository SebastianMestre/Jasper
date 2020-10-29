#pragma once

#include "block_allocator.hpp"

#include <type_traits>

#include <cstring>

// we use a block allocator to store pointers
// those pointers point to system-allocated objects
// this is a very dumb allocator
// We only allocate subtypes of Base because we do cleanup
// using virtual destructors
template<typename Base>
struct PolymorphicDumbAllocator {
	static_assert(
	    std::has_virtual_destructor<Base>::value,
	    "Base must have a virtual destructor");

	BlockAllocator m_allocator;

	PolymorphicDumbAllocator(int target_bytes_per_block)
	    : m_allocator {8, target_bytes_per_block} {}

	template <typename T>
	T* make() {
		static_assert(
		    std::is_base_of<Base, T>::value, "T must be a subtype of Base");
		auto storage = m_allocator.allocate(8);
		auto result = new T;
		memcpy(storage, &result, 8);
		return result;
	}

	~PolymorphicDumbAllocator() {

		auto clear_block = [&](uint8_t* block, int bytes) {
			uint8_t* end = block + bytes;
			block += 8; // skip next block pointer
			for (; block < end; block += m_allocator.m_bytes_per_slot) {
				Base* ptr;
				memcpy(&ptr, block, 8);
				delete ptr;
			}
		};

		int bytes_to_clear = m_allocator.m_bytes_used_in_block;
		for (uint8_t* it = m_allocator.m_data; it; memcpy(&it, it, 8)) {
			clear_block(it, bytes_to_clear);
			bytes_to_clear = m_allocator.m_bytes_per_block;
		}
    }
};
