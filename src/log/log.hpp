#include <string>

namespace Log {

void info(std::string const&);

void warning(char const*);

void error(char const*);

[[noreturn]] void fatal(std::string const&);

}
