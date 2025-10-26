#pragma once

#include <string>

#include "stream.hpp"

namespace Log {

InfoStream info();
WarningStream warning();
ErrorStream error();
FatalStream fatal();
InternalErrorStream internal_error();

void info(std::string const&);
void warning(std::string const&);
void error(std::string const&);
[[noreturn]] void fatal(std::string const&);
[[noreturn]] void internal_error(std::string const&);

// Missing case handlers for switch statements
[[noreturn]] void missing_case(std::string const& function_name, std::string const& ast_type);

} // namespace Log
