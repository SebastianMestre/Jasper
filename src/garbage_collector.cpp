#include "garbage_collector.hpp"

#include <algorithm>

namespace GarbageCollector {

GC::GC() {
	m_null = new Type::Null;
}

void GC::run() {
	for (auto* block : m_blocks) {
		block->m_visited = false;
	}

	for (auto* root : m_roots) {
		root->gc_visit();
	}

	for (auto*& block : m_blocks) {
		if (not block->m_visited) {
			delete block;
			block = nullptr;
		}
	}

	auto is_null = [&](Type::Value* p) { return p == nullptr; };

	m_blocks.erase(
	    std::remove_if(m_blocks.begin(), m_blocks.end(), is_null), m_blocks.end());
}

void GC::add_root(Type::Value* new_root) {
	m_roots.push_back(new_root);
}

Type::Object* GC::new_object() {
	auto result = new Type::Object;
	m_blocks.push_back(result);
	return result;
}

Type::List* GC::new_list() {
	auto result = new Type::List;
	m_blocks.push_back(result);
	return result;
}

} // namespace GarbageCollector
