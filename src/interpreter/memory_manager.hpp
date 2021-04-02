#pragma once

#include "garbage_collector.hpp"
#include "gc_ptr.hpp"
#include "value.hpp"

namespace Interpreter {

struct MemoryManager {
	Null* m_null;
	GC m_gc;

	MemoryManager();

	auto null() -> Null*;

	template <typename T, typename... Us>
	T* alloc_orphan(Us&&... us) {
		return new T(std::forward<Us>(us)...);
	}

	template <typename T, typename... Us>
	T* alloc_raw(Us&&... us) {
		return m_gc.adopt(alloc_orphan<T>(std::forward<Us>(us)...));
	}

	template <typename T, typename... Us>
	gc_ptr<T> alloc(Us&&... us) {
		return alloc_raw<T>(std::forward<Us>(us)...);
	}
};

} // namespace Interpreter
