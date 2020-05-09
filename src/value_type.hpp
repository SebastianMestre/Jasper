#pragma once

inline const char* value_type_string[] = {
	"Null",

	"Integer",
	"Float",
	"Boolean",
	"String",

	"List",
	"Object",
	"Dictionary",

	"Function"
};

enum class value_type {
	Null,

	Integer,
	Float,
	Boolean,
	String,

	List,
	Object,
	Dictionary,
	
	Function
};

