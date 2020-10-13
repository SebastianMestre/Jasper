#include "heap.hpp"

namespace Interpreter {
namespace GarbageCollector {

Heap::Heap() {
	m_allocators.emplace_back(16);
	m_allocators.emplace_back(32);
	m_allocators.emplace_back(128);
	m_allocators.emplace_back(512);
}

void Heap::free(Slot* s) {
	for (auto& allocator : m_allocators)
		if (allocator.contains(s))
			allocator.free(s);
}

Slot* Heap::allocate_impl(size_t size) {
	for (auto& allocator : m_allocators)
		if (allocator.slot_size() >= size)
			return allocator.allocate();
	return nullptr;
}

} // namespace GarbageCollector
} // namespace Interpreter
