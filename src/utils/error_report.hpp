#pragma once

#include <string>
#include <vector>

#include "source_location.hpp"
#include "string_view.hpp"

struct ErrorReport {
	std::string m_text;
	std::vector<ErrorReport> m_sub_errors;

	bool ok() const;
	void print(int d = 1) const;

	void add_sub_error(ErrorReport);
};

ErrorReport make_located_error(string_view text, SourceLocation location);
