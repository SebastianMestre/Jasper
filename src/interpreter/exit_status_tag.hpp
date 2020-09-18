#pragma once

#define EXIT_STATUS_TAGS \
	X(Ok) \
\
	X(ParseError) \
	X(StaticError) \
	X(TopLevelTypeError) \
\
	X(NullError) \
	X(TypeError) \
\
	X(ValueError) \
	X(Empty)

#define X(name) "name",
constexpr const char* exit_status_string[] = {
	EXIT_STATUS_TAGS
};
#undef X

#define X(name) name,
enum class ExitStatusTag {
	EXIT_STATUS_TAGS
};
#undef X

#undef EXIT_STATUS_TAGS
