#include "memory_manager.hpp"

#include "error.hpp"

namespace Interpreter {

MemoryManager::MemoryManager() {
	m_null = new Null;
}

Null* MemoryManager::null() {
	return m_null;
}

} // namespace Interpreter
