#include "frontend_context.hpp"

namespace Frontend {

SourceLocation Context::char_offset_to_location(int offset) const {

	int code_idx = 0;
	int line = 0;
	int col = 0;

	for (; code_idx < offset; ++code_idx) {
		if (source[code_idx] == '\n') {
			line += 1;
			col = 0;
		} else {
			col += 1;
		}
	}

	return {line, col};
}

} // namespace Frontend
