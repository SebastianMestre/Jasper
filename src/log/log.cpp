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
	info() << str << "\n";
}

void warning(std::string const& str) {
	warning() << str << "\n";
}

void error(std::string const& str) {
	error() << str << "\n";
}

void fatal(std::string const& str) {
	fatal() << str << "\n";
}

void internal_error(std::string const& str) {
	internal_error() << str << "\n";
}


void missing_case(std::string const& function_name, std::string const& ast_type) {
	internal_error() << "Unhandled case in " << function_name << " (" << ast_type << ")";
}

} // namespace Log
