#include "log.hpp"

#include <iostream>

namespace Log {

void warning(char const* str) {
	std::clog << "[ Warning ] " << str << "\n";
}

} // namespace Log
