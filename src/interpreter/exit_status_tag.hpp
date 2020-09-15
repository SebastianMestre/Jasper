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

enum class ExitStatusTag {
	Ok = 0,

	ParseError,
	StaticError,
	TopLevelTypeError,

	NullError,
	TypeError,
	ValueError,

	Empty,
};
