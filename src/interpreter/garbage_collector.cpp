#include "garbage_collector.hpp"

#include <algorithm>
#include <string>

namespace Interpreter {

GC::GC() {
}

GC::~GC() {
	for (auto* block : m_blocks) {
		delete block;
	}
}

void GC::sweep() {
	for (auto*& block : m_blocks) {
		if (not block->is_marked(m_generation)) {
			delete block;
			block = nullptr;
		}
	}

	auto is_null = [&](GcCell* p) { return p == nullptr; };

	m_blocks.erase(
	    std::remove_if(m_blocks.begin(), m_blocks.end(), is_null), m_blocks.end());
}

Variant* GC::new_variant_raw(InternedString constructor, Value v) {
	auto result = new Variant(constructor, v);
	m_blocks.push_back(result);
	return result;
}

Record* GC::new_record_raw(RecordType declarations) {
	auto result = new Record;
	result->m_value = std::move(declarations);
	m_blocks.push_back(result);
	return result;
}

Array* GC::new_list_raw(ArrayType elements) {
	auto result = new Array;
	result->m_value = std::move(elements);
	m_blocks.push_back(result);
	return result;
}

String* GC::new_string_raw(std::string s) {
	auto result = new String(std::move(s));
	m_blocks.push_back(result);
	return result;
}

Function* GC::new_function_raw(FunctionType def, CapturesType captures) {
	auto result = new Function(std::move(def), std::move(captures));
	m_blocks.push_back(result);
	return result;
}

Variable* GC::new_variable_raw(Value v) {
	auto result = new Variable(std::move(v));
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
