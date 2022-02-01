#pragma once

#include <string>

struct SourceLocation {
	int line, col;

	std::string to_string() const {
		return std::to_string(line + 1) + ":" + std::to_string(col + 1);
	}
};

struct SourceRange {
	SourceLocation start;
	SourceLocation end;
};
