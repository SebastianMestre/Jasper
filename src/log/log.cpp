#include "log.hpp"

#include <iostream>

namespace Log {

void warning(char const* str) {
	std::clog << "[ Warning ] " << str << "\n";
}

void error(char const* str) {
	std::cerr << "[ Error ] " << str << "\n";
}

void fatal(char const* str) {
	std::cerr << "[ Fatal Error ] " << str << "\n";
	exit(1);
}

} // namespace Log
