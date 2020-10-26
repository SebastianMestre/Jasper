#pragma once

#include <cstdint>

struct BlockAllocator {
	BlockAllocator(int bytes_per_slot, int target_bytes_per_block);
	uint8_t* allocate(int bytes);

  private:
	void new_block();
	bool block_is_full() const;

	int m_bytes_per_slot;
	int m_bytes_per_block;
	int m_bytes_used_in_block;
	uint8_t* m_data;
};
