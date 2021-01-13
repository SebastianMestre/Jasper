#include <string>

#include "fatal_stream.hpp"

namespace Log {

void info(std::string const&);
void warning(char const*);
void error(char const*);
[[noreturn]] void fatal(std::string const&);

}
