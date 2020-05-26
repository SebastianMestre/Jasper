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
private:
	bool m_function;
public:
	ast_vtype m_vtype {ast_vtype::Undefined};
	std::vector<ast_vtype> m_vargs;
	int m_wildcard;

	ValueType(ast_vtype type) : m_vtype{type} {}
	ValueType(bool is_function) : m_function{is_function} {}

	bool operator== (ast_vtype);
	bool operator!= (ast_vtype);
	bool operator!= (ValueType);
};