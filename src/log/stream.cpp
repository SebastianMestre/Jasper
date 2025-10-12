#include "stream.hpp"

#include <iostream>

namespace Log {

InfoStream::InfoStream()
    : LogStream {std::cout} {
	std::cout << "[ Info ] ";
}

WarningStream::WarningStream()
    : LogStream {std::clog} {
	std::clog << "[ Warning ] ";
}

ErrorStream::ErrorStream()
    : LogStream {std::cerr} {
	std::cerr << "[ Error ] ";
}

FatalStream::FatalStream()
    : LogStream {std::cerr} {
	std::cerr << "[ Fatal Error ] ";
}

FatalStream::~FatalStream() {
	std::cerr << "\n";
	exit(1);
}

InternalErrorStream::InternalErrorStream()
    : LogStream {std::cerr} {
	std::cerr << "[ Internal Error ] ";
}

InternalErrorStream::~InternalErrorStream() {
	std::cerr << "\n";
	exit(1);
}

} // namespace Log
