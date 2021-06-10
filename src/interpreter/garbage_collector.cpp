#include "garbage_collector.hpp"

#include "error.hpp"

#include <algorithm>
#include <string>

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
	for (auto* root : m_roots)
		root->visit();

	for (auto* val : m_blocks)
		if (val->m_cpp_refcount != 0)
			val->visit();
}

void GC::sweep() {
	for (auto*& block : m_blocks) {
		if (not block->m_visited) {
			delete block;
			block = nullptr;
		}
	}

	auto is_null = [&](GcCell* p) { return p == nullptr; };

	m_blocks.erase(
	    std::remove_if(m_blocks.begin(), m_blocks.end(), is_null), m_blocks.end());
}

void GC::sweep_all() {
	unmark_all();
	sweep();
}

void GC::add_root(GcCell* new_root) {
	m_roots.push_back(new_root);
}

gc_ptr<Variant> GC::new_variant(InternedString constructor, Value v) {
	auto result = new Variant(constructor, v);
	m_blocks.push_back(result);
	return result;
}

gc_ptr<Record> GC::new_record(RecordType declarations) {
	auto result = new Record;
	result->m_value = std::move(declarations);
	m_blocks.push_back(result);
	return result;
}

gc_ptr<Array> GC::new_list(ArrayType elements) {
	auto result = new Array;
	result->m_value = std::move(elements);
	m_blocks.push_back(result);
	return result;
}

gc_ptr<String> GC::new_string(std::string s) {
	return new_string_raw(std::move(s));
}

String* GC::new_string_raw(std::string s) {
	auto result = new String(std::move(s));
	m_blocks.push_back(result);
	return result;
}

gc_ptr<Function> GC::new_function(FunctionType def, CapturesType captures) {
	auto result = new Function(std::move(def), std::move(captures));
	m_blocks.push_back(result);
	return result;
}

gc_ptr<Error> GC::new_error(std::string s) {
	auto result = new Error(std::move(s));
	m_blocks.push_back(result);
	return result;
}

gc_ptr<Reference> GC::new_reference(Value v) {
	auto result = new Reference(std::move(v));
	m_blocks.push_back(result);
	return result;
}

VariantConstructor* GC::new_variant_constructor_raw(InternedString constructor) {
	auto result = new VariantConstructor(constructor);
	m_blocks.push_back(result);
	return result;
}

RecordConstructor* GC::new_record_constructor_raw(std::vector<InternedString> keys) {
	auto result = new RecordConstructor(std::move(keys));
	m_blocks.push_back(result);
	return result;
}

} // namespace Interpreter
