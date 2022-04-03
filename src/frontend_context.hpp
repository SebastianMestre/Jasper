#pragma once

#include <string>
#include "./utils/source_location.hpp"

namespace Frontend {

struct Context {
	std::string source;

	SourceLocation char_offset_to_location(int offset) const;
};

} // namespace Frontend
