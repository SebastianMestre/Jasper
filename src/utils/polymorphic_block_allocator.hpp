#pragma once

#include "block_allocator.hpp"

#include <type_traits>

#include <cstring>

template<typename Base>
struct PolymorphicBlockAllocator {
	static_assert(
	    std::has_virtual_destructor<Base>::value,
	    "Base must have a virtual destructor");

	BlockAllocator m_allocator;

	PolymorphicBlockAllocator(int bytes_per_slot, int target_bytes_per_block)
	    : m_allocator {bytes_per_slot, target_bytes_per_block} {}

	template <typename T>
	T* make() {
		static_assert(
		    std::is_base_of<Base, T>::value, "T must be a subtype of Base");

		auto data = m_allocator.allocate(sizeof(T));
		return new(data) T();
	}

	~PolymorphicBlockAllocator(){
		auto clear_block = [&](uint8_t* block, int bytes) {
			uint8_t* end = block + bytes;
			block += 8; // skip next block pointer
			for (; block < end; block += m_allocator.m_bytes_per_slot) {
				reinterpret_cast<Base*>(block)->~Base();
			}

		};

		int bytes_to_clear = m_allocator.m_bytes_used_in_block;
		for (uint8_t* it = m_allocator.m_data; it; memcpy(&it, it, 8)) {
			clear_block(it, bytes_to_clear);
			bytes_to_clear = m_allocator.m_bytes_per_block;
		}
	}
};
