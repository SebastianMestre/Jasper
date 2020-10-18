#include "block.hpp"

#include <cassert>

namespace Interpreter {
namespace GarbageCollector {

struct FreeList : public Slot {
	FreeList* next;
};

Block::Block(int slot_size)
    : m_block {std::make_unique<Buffer>()}
    , m_slot_size {slot_size} {
	m_block->fill(0);

	assert(slot_size >= sizeof(FreeList));
	for (int i = 0; i < slot_count(); ++i) {
		auto node = static_cast<FreeList*>(slot(i));
		node->next = (i == slot_count() - 1)
		                 ? nullptr
		                 : static_cast<FreeList*>(slot(i + 1));
	}
	m_freelist = static_cast<FreeList*>(slot(0));
}

Slot* Block::allocate() {
	if (!m_freelist)
		return nullptr;
	return std::exchange(m_freelist, m_freelist->next);
}

void Block::free(Slot* s) {
	auto node = reinterpret_cast<FreeList*>(s);
	node->next = m_freelist;
	m_freelist = node;
}

bool Block::contains(Slot* s) {
	return slot(0) <= s && s < slot(slot_count());
}

int Block::slot_count() const {
	return block_size / m_slot_size;
}

int Block::slot_size() const {
	return m_slot_size;
}

Slot* Block::slot(size_t index) {
	return reinterpret_cast<Slot*>(&(*m_block)[index * m_slot_size]);
}

} // namespace GarbageCollector
} // namespace Interpreter
