#include <string>

namespace Log {

void warning(char const*);

void error(char const*);

[[noreturn]] void fatal(std::string const&);

}
