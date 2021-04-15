#pragma once

#include <string>
#include <vector>

#include "source_location.hpp"
#include "utils/string_view.hpp"

struct ErrorReport {
	std::string m_text;
	std::vector<ErrorReport> m_sub_errors;

	bool ok();
	void print(int d = 1);
};

ErrorReport make_located_error(string_view text, SourceLocation location);
