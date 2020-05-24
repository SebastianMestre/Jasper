#pragma once

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
	NativeFunction,
	
	TypeError,
	Void,
	Undefined,
	Wildcard
};