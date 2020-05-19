#include "value.hpp"
#include "environment.hpp"
#include <cassert>

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
    Type::Array* array = static_cast<Type::Array*>(unboxed(v[0]));
    for (unsigned int i = 1; i < v.size(); i++) {
        array->m_value.push_back(unboxed(v[i]));
    }
    return array;
}

// array_extend(arr1, arr2) appends the values in arr2 to
// arr1
Type::Value* array_extend(Type::ArrayType v, Type::Environment& e) {
    // TODO proper error handling
    assert(v.size() == 2);
    Type::Array* arr1 = static_cast<Type::Array*>(unboxed(v[0]));
    Type::Array* arr2 = static_cast<Type::Array*>(unboxed(v[1]));
    arr1->m_value.insert(
        arr1->m_value.end(), 
        arr2->m_value.begin(), 
        arr2->m_value.end());
    return arr1;
}

// size(array) returns the size of the array
/* Type::Value* size(Type::ArrayType v, Type::Environment& e) {
    // TODO proper error handling
    assert(v.size() == 1);
    Type::Array* array = static_cast<Type::Array*>(unboxed(v[0]));
    int size = array->m_value.size();
    return &Type::Integer(std::move(size));
} */


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
}