#pragma once

#include "slot.hpp"

#include <array>
#include <memory>

namespace Interpreter {
namespace GarbageCollector {

struct FreeList;

struct Block {
	using Byte = unsigned char;

	constexpr static int block_size = 4 * 1024; // 4kiB
	using Buffer = std::array<Byte, block_size>;

	explicit Block(int slot_size);

	Slot* allocate();
	void free(Slot* s);
	bool contains(Slot* s);

	int slot_count() const;
	int slot_size() const;

  private:
	Slot* slot(size_t index);

	FreeList* m_freelist;
	std::unique_ptr<Buffer> m_block;
	int m_slot_size;
};

} // namespace GarbageCollector
} // namespace Interpreter
