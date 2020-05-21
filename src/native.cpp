#include "value.hpp"
#include "environment.hpp"
#include "value_type.hpp"
#include <cassert>
#include <sstream>

// print(...) prints the values or references in ...
Type::Value* print (Type::ArrayType v, Type::Environment& e) {
    for (auto value : v) {
        Type::print(value);
    } 
    return e.null();
}

// array_append(arr, ...) appends the values or references
// in ... to the array
Type::Value* array_append(Type::ArrayType v, Type::Environment& e) {
    // TODO proper error handling
    assert(v.size() > 0);
    assert(unboxed(v[0])->type() == value_type::Array);
    Type::Array* array = static_cast<Type::Array*>(unboxed(v[0]));
    for (unsigned int i = 1; i < v.size(); i++) {
        array->m_value.push_back(v[i]);
    }
    return array;
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Type::Value* array_extend(Type::ArrayType v, Type::Environment& e) {
    // TODO proper error handling
    assert(v.size() == 2);
    assert(unboxed(v[0])->type() == value_type::Array);
    assert(unboxed(v[1])->type() == value_type::Array);
    Type::Array* arr1 = static_cast<Type::Array*>(unboxed(v[0]));
    Type::Array* arr2 = static_cast<Type::Array*>(unboxed(v[1]));
    arr1->m_value.insert(
        arr1->m_value.end(), 
        arr2->m_value.begin(), 
        arr2->m_value.end());
    return arr1;
}

// size(array) returns the size of the array
Type::Value* size(Type::ArrayType v, Type::Environment& e) {
    // TODO proper error handling
    assert(v.size() == 1);
    assert(unboxed(v[0])->type() == value_type::Array);
    Type::Array* array = static_cast<Type::Array*>(unboxed(v[0]));
    return e.new_integer(array->m_value.size());
} 

// array_join(array, string) returns a string with
// the array values separated by the string element
Type::Value* array_join(Type::ArrayType v, Type::Environment& e) {
    // TODO proper error handling
    assert(v.size() == 2);
    assert(unboxed(v[0])->type() == value_type::Array);
    assert(unboxed(v[1])->type() == value_type::String);
    Type::Array* array = static_cast<Type::Array*>(unboxed(v[0]));
    Type::String* string = static_cast<Type::String*>(unboxed(v[1]));
    std::stringstream result;
    for (unsigned int i = 0; i < array->m_value.size(); i++) {
        // TODO make it more general
        auto* value = unboxed(array->m_value[i]);
    
        assert(value->type() == value_type::Integer);

        result << static_cast<Type::Integer*>(value)->m_value;
        if (i < array->m_value.size()-1)
            result << string->m_value;
    }
    return e.new_string(result.str());
}


void declare_native_functions(Type::Environment& env) {
    env.declare(
        "print",
        env.new_native_function(
            static_cast<Type::NativeFunctionType*>(&print)));

    env.declare(
        "array_append",
        env.new_native_function(
            static_cast<Type::NativeFunctionType*>(&array_append)));

    env.declare(
        "array_extend",
        env.new_native_function(
            static_cast<Type::NativeFunctionType*>(&array_extend)));

    env.declare(
        "size",
        env.new_native_function(
            static_cast<Type::NativeFunctionType*>(&size)));

    env.declare(
        "array_join",
        env.new_native_function(
            static_cast<Type::NativeFunctionType*>(&array_join)));
}