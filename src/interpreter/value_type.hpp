#pragma once

constexpr const char* value_type_string[] = {
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

enum class ValueTag {
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
