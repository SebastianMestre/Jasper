#pragma once

constexpr const char* test_type_string[] = {
	"Ok",

	"ParseError",
	"TopLevelTypeError",

	"NullResult",
	"TypeError",
	"ValueError",

	"Empty",
};

enum class test_type {
	Ok,

	ParseError,
	TopLevelTypeError,

	NullResult,
	TypeError,
	ValueError,

	Empty,
};
