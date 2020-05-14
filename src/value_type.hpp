#pragma once

inline const char* value_type_string[] = {
	"Null",

	"Integer",
	"Float",
	"Boolean",
	"String",
	"Error",

	"List",
	"Object",
	"Dictionary",

	"Function",
	"NativeFunction",
};

enum class value_type {
	Null,

	Integer,
	Float,
	Boolean,
	String,
	Error,

	List,
	Object,
	Dictionary,
	
	Function,
	NativeFunction,
};

