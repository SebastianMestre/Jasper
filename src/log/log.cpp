#include "log.hpp"

#include <iostream>

namespace Log {

InfoStream info() {
	return InfoStream();
}

WarningStream warning() {
	return WarningStream();
}

ErrorStream error() {
	return ErrorStream();
}

FatalStream fatal() {
	return FatalStream();
}

InternalErrorStream internal_error() {
	return InternalErrorStream();
}


void info(std::string const& str) {
	std::clog << "[ Info ] " << str << "\n";
}

void warning(std::string const& str) {
	std::clog << "[ Warning ] " << str << "\n";
}

void error(std::string const& str) {
	std::cerr << "[ Error ] " << str << "\n";
}

void fatal(std::string const& str) {
	std::cerr << "[ Fatal Error ] " << str << "\n";
	exit(1);
}

void internal_error(std::string const& str) {
	std::cerr << "[ Internal Error ] " << str << "\n";
	exit(1);
}

} // namespace Log
