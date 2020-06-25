#include "value.hpp"
#include "environment.hpp"
#include "value_type.hpp"

#include <sstream>
#include <iostream>

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
    assert(unboxed(v[0])->type() == value_type::Array);
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

Type::Value* dummy(Type::ArrayType v, Type::Environment& e){
	return e.null();
}

Type::Value* value_add(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		return e.new_integer(
		    static_cast<Type::Integer*>(lhs_val)->m_value
		    + static_cast<Type::Integer*>(rhs_val)->m_value);
	case value_type::Float:
		return e.new_float(
		    static_cast<Type::Float*>(lhs_val)->m_value
		    + static_cast<Type::Float*>(rhs_val)->m_value);
	case value_type::String:
		return e.new_string(
		    static_cast<Type::String*>(lhs_val)->m_value
		    + static_cast<Type::String*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Type::Value* value_sub(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		return e.new_integer(
		    static_cast<Type::Integer*>(lhs_val)->m_value
		    - static_cast<Type::Integer*>(rhs_val)->m_value);
	case value_type::Float:
		return e.new_float(
		    static_cast<Type::Float*>(lhs_val)->m_value
		    - static_cast<Type::Float*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't add values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Type::Value* value_mul(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		return e.new_integer(
		    static_cast<Type::Integer*>(lhs_val)->m_value
		    * static_cast<Type::Integer*>(rhs_val)->m_value);
	case value_type::Float:
		return e.new_float(
		    static_cast<Type::Float*>(lhs_val)->m_value
		    * static_cast<Type::Float*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't multiply values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Type::Value* value_div(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());
	switch (lhs_val->type()) {
	case value_type::Integer:
		return e.new_integer(
		    static_cast<Type::Integer*>(lhs_val)->m_value
		    / static_cast<Type::Integer*>(rhs_val)->m_value);
	case value_type::Float:
		return e.new_float(
		    static_cast<Type::Float*>(lhs_val)->m_value
		    / static_cast<Type::Float*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't divide values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Type::Value* value_logicand(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == value_type::Boolean
	    and rhs_val->type() == value_type::Boolean)
		return e.new_boolean(
		    static_cast<Type::Boolean*>(lhs_val)->m_value
		    and static_cast<Type::Boolean*>(rhs_val)->m_value);
	std::cerr << "ERROR: logical and operator not defined for types "
	          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_type_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Type::Value* value_logicor(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == value_type::Boolean
	    and rhs_val->type() == value_type::Boolean)
		return e.new_boolean(
		    static_cast<Type::Boolean*>(lhs_val)->m_value
		    or static_cast<Type::Boolean*>(rhs_val)->m_value);
	std::cerr << "ERROR: logical or operator not defined for types "
	          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_type_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Type::Value* value_logicxor(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	if (lhs_val->type() == value_type::Boolean
	    and rhs_val->type() == value_type::Boolean)
		return e.new_boolean(
		    static_cast<Type::Boolean*>(lhs_val)->m_value
		    != static_cast<Type::Boolean*>(rhs_val)->m_value);
	std::cerr << "ERROR: exclusive or operator not defined for types "
	          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
	          << value_type_string[static_cast<int>(rhs_val->type())];
	assert(0);
}

Type::Value* value_equals(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());

	switch (lhs_val->type()) {
	case value_type::Null:
		return e.new_boolean(true);
	case value_type::Integer:
		return e.new_boolean(
		    static_cast<Type::Integer*>(lhs_val)->m_value
		    == static_cast<Type::Integer*>(rhs_val)->m_value);
	case value_type::Float:
		return e.new_boolean(
		    static_cast<Type::Float*>(lhs_val)->m_value
		    == static_cast<Type::Float*>(rhs_val)->m_value);
	case value_type::String:
		return e.new_boolean(
		    static_cast<Type::String*>(lhs_val)->m_value
		    == static_cast<Type::String*>(rhs_val)->m_value);
	case value_type::Boolean:
		return e.new_boolean(
		    static_cast<Type::Boolean*>(lhs_val)->m_value
		    == static_cast<Type::Boolean*>(rhs_val)->m_value);
	default: {
		std::cerr << "ERROR: can't compare equality of types "
		          << value_type_string[static_cast<int>(lhs_val->type())] << " and "
		          << value_type_string[static_cast<int>(rhs_val->type())];
		assert(0);
	}
	}
}

Type::Value* value_less(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	assert(lhs_val->type() == rhs_val->type());

	switch (lhs_val->type()) {
	case value_type::Integer:
		return e.new_boolean(
		    static_cast<Type::Integer*>(lhs_val)->m_value
		    < static_cast<Type::Integer*>(rhs_val)->m_value);
	case value_type::Float:
		return e.new_boolean(
		    static_cast<Type::Float*>(lhs_val)->m_value
		    < static_cast<Type::Float*>(rhs_val)->m_value);
	case value_type::String:
		return e.new_boolean(
		    static_cast<Type::String*>(lhs_val)->m_value
		    < static_cast<Type::String*>(rhs_val)->m_value);
	default:
		std::cerr << "ERROR: can't compare values of type "
		          << value_type_string[static_cast<int>(lhs_val->type())];
		assert(0);
	}
}

Type::Value* value_assign(Type::ArrayType v, Type::Environment& e) {
	auto* lhs = v[0];
	auto* rhs = v[1];
	auto* lhs_val = unboxed(lhs);
	auto* rhs_val = unboxed(rhs);

	// TODO: proper error handling
	assert(lhs->type() == value_type::Reference);
	// NOTE: copied by reference, matters if rhs is actually a reference
	// TODO: change in another pr, perhaps adding Environment::copy_value?
	static_cast<Type::Reference*>(lhs)->m_value = rhs_val;
	return e.null();
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

	env.declare("+", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_add)));
	env.declare("-", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_sub)));
	env.declare("*", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_mul)));
	env.declare("/", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_div)));
	env.declare("<", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_less)));
	env.declare("=", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_assign)));
	env.declare("==", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_equals)));
	env.declare("^^", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_logicxor)));
	env.declare("&&", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_logicand)));
	env.declare("||", env.new_native_function(static_cast<Type::NativeFunctionType*>(&value_logicor)));
}
