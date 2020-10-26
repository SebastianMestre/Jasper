#pragma once

#include <cassert>
#include <cstdint>

struct BlockAllocator {
	friend struct AutomaticBlockAllocator;

	BlockAllocator(int bytes_per_slot, int target_bytes_per_block);
	uint8_t* allocate(int bytes) {
		assert(bytes <= m_bytes_per_slot);
		if (block_is_full())
			new_block();
		auto const result = m_data + m_bytes_used_in_block;
		m_bytes_used_in_block += m_bytes_per_slot;
		return result;
	}

  private:
	void new_block();
	bool block_is_full() const {
		assert(m_bytes_used_in_block <= m_bytes_per_block);
		return m_bytes_used_in_block == m_bytes_per_block;
	}

	int m_bytes_per_slot;
	int m_bytes_per_block;
	int m_bytes_used_in_block;
	uint8_t* m_data;
};
