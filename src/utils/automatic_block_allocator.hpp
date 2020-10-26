#pragma once

#include "block_allocator.hpp"

// #include <cstdio>
#include <cstring>

// a fixed size block allocator
// calls destructor on allocated objects when destructed
struct AutomaticBlockAllocator {

	AutomaticBlockAllocator(int bytes_per_object, int target_bytes_per_block);

	template<typename T>
	T* make() {
		// fprintf(stderr, "%s -- %lu\n", __PRETTY_FUNCTION__, sizeof(T));

		constexpr int bytes_per_object = sizeof(T);
		int bytes_per_slot =
		    bytes_per_slot_from_bytes_per_object(bytes_per_object);

		auto data = m_allocator.allocate(bytes_per_slot);

		Destructor* dtor = +[](void* ptr) { static_cast<T*>(ptr)->~T(); };
		memcpy(data, &dtor, bytes_per_slot_header);
		data += bytes_per_slot_header;

		return new(data) T;
	}

  private:
	// a slot header contains a function pointer.
	// we assume that an fptr takes up 8 bytes.
	static constexpr int bytes_per_slot_header = 8;

  	static int bytes_per_slot_from_bytes_per_object(int bytes_per_object);

	using Destructor = void(void*);
	BlockAllocator m_allocator;
};
