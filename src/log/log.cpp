#include "log.hpp"

#include <iostream>

namespace Log {

void info(std::string const& str) {
	std::clog << "[ Info ] " << str << "\n";
}


void warning(char const* str) {
	std::clog << "[ Warning ] " << str << "\n";
}

void error(char const* str) {
	std::cerr << "[ Error ] " << str << "\n";
}

void fatal(std::string const& str) {
	std::cerr << "[ Fatal Error ] " << str << "\n";
	exit(1);
}

} // namespace Log
