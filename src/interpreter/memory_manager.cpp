#include "memory_manager.hpp"

#include "error.hpp"

namespace Interpreter {

MemoryManager::MemoryManager() {
	m_null = new Null;
}

Null* MemoryManager::null() {
	return m_null;
}

/*
template<typename T, typename... U>
T* alloc_raw(U&&... us) {
	auto result = new T(std::forward<U>(...us));
	return m_gc.adopt(result);
}

template<typename T, typename U>
gc_ptr<T> alloc(U&&... us) {
	return alloc_raw<T>(std::forward<U>(...us));
}
*/

gc_ptr<Variant> MemoryManager::new_variant(InternedString constructor, Value* v) {
	auto result = new Variant(constructor, v);
	return m_gc.adopt(result);
}

gc_ptr<Record> MemoryManager::new_record(RecordType declarations) {
	auto result = new Record(std::move(declarations));
	return m_gc.adopt(result);
}

gc_ptr<Array> MemoryManager::new_list(ArrayType elements) {
	auto result = new Array(std::move(elements));
	return m_gc.adopt(result);
}

gc_ptr<Integer> MemoryManager::new_integer(int i) {
	return new_integer_raw(i);
}

Integer* MemoryManager::new_integer_raw(int i) {
	auto result = new Integer(i);
	return m_gc.adopt(result);
}

gc_ptr<Float> MemoryManager::new_float(float f) {
	return new_float_raw(f);
}

Float* MemoryManager::new_float_raw(float f) {
	auto result = new Float(f);
	return m_gc.adopt(result);
}

gc_ptr<Boolean> MemoryManager::new_boolean(bool b) {
	return new_boolean_raw(b);
}

Boolean* MemoryManager::new_boolean_raw(bool b) {
	auto result = new Boolean(b);
	return m_gc.adopt(result);
}

gc_ptr<String> MemoryManager::new_string(std::string s) {
	return new_string_raw(std::move(s));
}

String* MemoryManager::new_string_raw(std::string s) {
	auto result = new String(std::move(s));
	return m_gc.adopt(result);
}

gc_ptr<Function> MemoryManager::new_function(FunctionType def, CapturesType captures) {
	auto result = new Function(std::move(def), std::move(captures));
	return m_gc.adopt(result);
}

gc_ptr<NativeFunction> MemoryManager::new_native_function(NativeFunctionType* fptr) {
	auto result = new NativeFunction(fptr);
	return m_gc.adopt(result);
}

gc_ptr<Error> MemoryManager::new_error(std::string s) {
	auto result = new Error(std::move(s));
	return m_gc.adopt(result);
}

gc_ptr<Reference> MemoryManager::new_reference(Value* v) {
	auto result = new Reference(std::move(v));
	return m_gc.adopt(result);
}

VariantConstructor* MemoryManager::new_variant_constructor_raw(InternedString constructor) {
	auto result = new VariantConstructor(constructor);
	return m_gc.adopt(result);
}

RecordConstructor* MemoryManager::new_record_constructor_raw(std::vector<InternedString> keys) {
	auto result = new RecordConstructor(std::move(keys));
	return m_gc.adopt(result);
}

} // namespace Interpreter
