#include "fatal_stream.hpp"

#include <iostream>

namespace Log {

#define INSTANCE_SHIFT_OPERATOR(type)                                          \
	FatalStream& FatalStream::operator<<(type in) {                            \
		std::cerr << in;                                                       \
		return *this;                                                          \
	}

INSTANCE_SHIFT_OPERATOR(std::string const&)
INSTANCE_SHIFT_OPERATOR(int)
INSTANCE_SHIFT_OPERATOR(float)
INSTANCE_SHIFT_OPERATOR(char)

FatalStream::FatalStream() {
	std::cerr << "[ Fatal Error ] ";
}

FatalStream::~FatalStream() {
	std::cerr << "\n";
	exit(1);
}

} // namespace Log
