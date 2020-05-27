#pragma once

#include <vector>
#include <unordered_map>

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
	
	Function,
	Reference,
	
	TypeError,
	Void,
	Undefined,
	Wildcard
};

namespace TypeChecker {

enum class type_type {
	builtin,
	error,
	reference,
	function,
	sum,
	product,
	polymorphic,
	applied,
	wildcard,
};

struct Type {
	type_type m_type;
};

struct Wildcard : Type {
	int id;
};

struct Polymorphic : Type {
	std::vector<int> forall_ids;
	Type* m_base;
};

struct Applied : Type {
	Polymorphic* m_base;
	std::vector<Type*> args;
};

struct Error : Type {
	// informacion para printear
};

struct Sum : Type {
	std::vector<Type*> m_types;
};

struct Product : Type {
	std::vector<Type*> m_types;
};

struct Builtin : Type {
	void* def; // TODO: que va aca?
};

struct Reference : Type {
	Type* m_base;
};

struct Function : Type {
	std::vector<Type*> arg_types;
	Type* return_type;
};

/* static std::unordered_map<std::string, Type*> type_table = {
	{"int", new Builtin{}},
	{"runtime_error", new Builtin{}},
	{"unit", new Builtin{}},
	{"unit", new Builtin{}},
}; */

}