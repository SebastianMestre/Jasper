#include "garbage_collector.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace Interpreter {

GC::GC() {
	m_null = new Null;
}

GC::~GC(){
	delete m_null;
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
	mark_roots();
	sweep();
}

void GC::add_root(Value* new_root) {
	m_roots.push_back(new_root);
}



Null* GC::null() {
	return m_null;
}

Object* GC::new_object(ObjectType declarations) {
	auto result = new Object;
	result->m_value = std::move(declarations);
	m_blocks.push_back(result);
	return result;
}

Dictionary* GC::new_dictionary(ObjectType declarations) {
	auto result = new Dictionary;
	result->m_value = std::move(declarations);
	m_blocks.push_back(result);
	return result;
}

Array* GC::new_list(ArrayType elements) {
	auto result = new Array;
	result->m_value = std::move(elements);
	m_blocks.push_back(result);
	return result;
}

Integer* GC::new_integer(int i) {
	auto result = new Integer(i);
	m_blocks.push_back(result);
	return result;
}

Float* GC::new_float(float f) {
	auto result = new Float(f);
	m_blocks.push_back(result);
	return result;
}

Boolean* GC::new_boolean(bool b) {
	auto result = new Boolean(b);
	m_blocks.push_back(result);
	return result;
}

String* GC::new_string(std::string s) {
	auto result = new String(std::move(s));
	m_blocks.push_back(result);
	return result;
}

Function* GC::new_function(FunctionType def, ObjectType captures) {
	auto result = new Function(std::move(def), std::move(captures));
	m_blocks.push_back(result);
	return result;
}

NativeFunction* GC::new_native_function(NativeFunctionType* fptr) {
	auto result = new NativeFunction(fptr);
	m_blocks.push_back(result);
	return result;
}

Error* GC::new_error(std::string s) {
	auto result = new Error(std::move(s));
	m_blocks.push_back(result);
	return result;
}

Reference* GC::new_reference(Value* v) {
	auto result = new Reference(std::move(v));
	m_blocks.push_back(result);
	return result;
}

} // namespace Interpreter
