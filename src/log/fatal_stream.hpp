#include <string>

#include "log_stream.hpp"

namespace Log {

struct FatalStream : public LogStream {
	FatalStream();
	[[noreturn]] ~FatalStream();
};

} // namespace Log
