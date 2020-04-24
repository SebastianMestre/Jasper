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

Type::Null* GC::null() {
	return m_null;
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

Type::Integer* GC::new_integer(int i) {
	auto result = new Type::Integer(i);
	m_blocks.push_back(result);
	return result;
}

Type::Float* GC::new_float(float f) {
	auto result = new Type::Float(f);
	m_blocks.push_back(result);
	return result;
}

Type::String* GC::new_string(std::string s) {
	auto result = new Type::String(std::move(s));
	m_blocks.push_back(result);
	return result;
}

Type::Function* GC::new_function(Type::FunctionType def, Type::Scope* scope) {
	auto result = new Type::Function(std::move(def), std::move(scope));
	m_blocks.push_back(result);
	return result;
}

Type::Error* GC::new_error(std::string s) {
	auto result = new Type::Error(std::move(s));
	m_blocks.push_back(result);
	return result;
}

} // namespace GarbageCollector
