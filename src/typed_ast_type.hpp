#pragma once

#include <vector>

// runtime type
enum class ast_vtype {
    Null,
	
	Integer,
	Float,
	Boolean,
	String,
	Error,

	Array,
	Object,
	Dictionary,
	
	TypeError,
	Void,
	Undefined
};

class ValueType {
public:
	bool m_function;
	ast_vtype m_vtype {ast_vtype::Undefined};
	std::vector<ValueType> m_vargs;
	int m_wildcard;

	ValueType(ast_vtype type) : m_vtype{type} {}
	ValueType(bool is_function) : m_function{is_function} {}

	bool operator== (ast_vtype);
	bool operator== (ValueType);
	bool operator!= (ast_vtype);
	bool operator!= (ValueType);
	bool operator= (ValueType);
};