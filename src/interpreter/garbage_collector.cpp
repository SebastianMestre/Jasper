#include "garbage_collector.hpp"

#include "value.hpp"

#include <algorithm>
#include <string>
#include <type_traits>

namespace Interpreter {

GC::GC() {
}

GC::~GC() {
	sweep_all();
}

void GC::unmark_all() {
	for (auto* block : m_blocks) {
		block->m_visited = false;
	}
}

void GC::mark_roots() {
	for (auto* root : m_roots) {
		gc_visit(root);
	}

	for (auto* val : m_blocks) {
		if (val->m_cpp_refcount != 0) {
			gc_visit(val);
		}
	}
}

void GC::sweep() {
	for (auto*& block : m_blocks) {
		if (not block->m_visited) {
			delete block;
			block = nullptr;
		}
	}

	auto is_null = [&](Value* p) { return p == nullptr; };

	m_blocks.erase(
	    std::remove_if(m_blocks.begin(), m_blocks.end(), is_null), m_blocks.end());
}

void GC::sweep_all() {
	unmark_all();
	sweep();
}

void GC::add_root(Value* new_root) {
	m_roots.push_back(new_root);
}

void GC::adopt_impl(Value* value) {
	m_blocks.push_back(value);
}

} // namespace Interpreter
