#include "fatal_stream.hpp"

#include <iostream>

namespace Log {

FatalStream::FatalStream()
    : LogStream {std::cerr} {
	std::cerr << "[ Fatal Error ] ";
}

FatalStream::~FatalStream() {
	std::cerr << "\n";
	exit(1);
}

} // namespace Log
