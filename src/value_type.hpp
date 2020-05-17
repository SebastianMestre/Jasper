#pragma once

inline const char* value_type_string[] = {
	"Null",

	"Integer",
	"Float",
	"Boolean",
	"String",
	"Error",

	"Array",
	"Object",
	"Dictionary",

	"Function",
	"NativeFunction",

	"Reference",
};

enum class value_type {
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
	NativeFunction,

	Reference,
};

