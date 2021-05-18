#pragma once

#define EXIT_STATUS                                                            \
	X(Ok)                                                                      \
                                                                               \
	X(ParseError)                                                              \
	X(StaticError)                                                             \
	X(TopLevelTypeError)                                                       \
                                                                               \
	X(NullError)                                                               \
	X(TypeError)                                                               \
                                                                               \
	X(ValueError)                                                              \
	X(Empty)

#define X(name) #name,
constexpr const char* exit_status_string[] = {EXIT_STATUS};
#undef X

#define X(name) name,
enum class ExitStatus { EXIT_STATUS };
#undef X

#undef EXIT_STATUS
