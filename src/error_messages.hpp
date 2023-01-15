#pragma once

#include "./utils/error_report.hpp"

#include <string>

namespace Frontend {
struct Context;
}

struct Token;

ErrorReport make_unexpected_error(
    Frontend::Context const& file_context,
	std::string expected,
	Token const* found);

ErrorReport make_unexpected_error_with_open_brace(
    Frontend::Context const& file_context,
    Token const* opening,
    std::string expected,
    Token const* found);

ErrorReport make_located_error(
    Frontend::Context const& file_context,
    std::string text,
    Token const* token,
    std::string comment);
