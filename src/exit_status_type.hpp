#pragma once

constexpr const char* exit_status_type_string[] = {
	"Ok",

	"ParseError",
	"TopLevelTypeError",

	"NullError",
	"TypeError",
	"ValueError",

	"Empty",
};

enum class exit_status_type {
	Ok = 0,

	ParseError,
	TopLevelTypeError,

	NullError,
	TypeError,
	ValueError,

	Empty,
};
