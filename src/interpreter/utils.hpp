#pragma once

#include <cassert>

#include "value.hpp"

namespace Interpreter {

template<typename T>
struct type_data;

template<> struct type_data<Integer> { static constexpr auto tag = ValueTag::Integer; };
template<> struct type_data<Float> { static constexpr auto tag = ValueTag::Float; };
template<> struct type_data<String> { static constexpr auto tag = ValueTag::String; };
template<> struct type_data<Array> { static constexpr auto tag = ValueTag::Array; };
template<> struct type_data<Record> { static constexpr auto tag = ValueTag::Record; };
template<> struct type_data<Variant> { static constexpr auto tag = ValueTag::Variant; };
template<> struct type_data<Function> { static constexpr auto tag = ValueTag::Function; };
template<> struct type_data<NativeFunction> { static constexpr auto tag = ValueTag::NativeFunction; };
template<> struct type_data<Reference> { static constexpr auto tag = ValueTag::Reference; };
template<> struct type_data<VariantConstructor> { static constexpr auto tag = ValueTag::VariantConstructor; };
template<> struct type_data<RecordConstructor> { static constexpr auto tag = ValueTag::RecordConstructor; };

Handle value_of(Handle value);

template<typename T>
T* as(Handle h) {
	static_assert(std::is_base_of<Value, T>::value, "T is not a subclass of Value");
	assert(h.type() == type_data<T>::tag);
	T* result = h.get_cast<T>();
	assert(result);
	return result;
}

template<typename T>
T* value_as(Handle h) {
	static_assert(std::is_base_of<Value, T>::value, "T is not a subclass of Value");
	return as<T>(value_of(h));
}

} // namespace Interpreter
