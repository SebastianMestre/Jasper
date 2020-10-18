#pragma once

#include "block.hpp"

#include <vector>

namespace Interpreter {
namespace GarbageCollector {

struct Heap {
	Heap();

	template <typename T, typename... Args>
	T* allocate(Args&&... args) {
		static_assert(
		    std::is_base_of<Slot, T>::value,
		    "You must inherit from Slot to allocate from here");
		Slot* memory = allocate_impl(sizeof(T));
		return new (memory) T(std::forward<Args>(args)...);
	}

	void free(Slot* s);

  private:
	Slot* allocate_impl(size_t size);

	std::vector<Block> m_allocators;
};

struct Integer : public Slot {
	int m_value;
	Integer() {}
	Integer(int value)
	    : m_value(value) {}
	int value() const {
		return m_value;
	}
};

} // namespace GarbageCollector
} // namespace Interpreter
