#include <string>

namespace Log {

struct FatalStream {
	FatalStream();
	[[noreturn]] ~FatalStream();

	FatalStream& operator<<(std::string const&);
	FatalStream& operator<<(int);
	FatalStream& operator<<(float);
	FatalStream& operator<<(char);
};

} // namespace Log
