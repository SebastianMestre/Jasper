#include "basic_stream.hpp"

namespace Log {

struct InfoStream : public LogStream {
	InfoStream();
};

struct WarningStream : public LogStream {
	WarningStream();
};

struct ErrorStream : public LogStream {
	ErrorStream();
};

struct FatalStream : public LogStream {
	FatalStream();
	[[noreturn]] ~FatalStream();
};

} // namespace Log
