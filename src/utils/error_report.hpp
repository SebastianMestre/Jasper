#pragma once

#include <string>
#include <vector>

#include "source_location.hpp"
#include "string_view.hpp"

struct ErrorReport {
	std::vector<std::string> m_lines;
	std::vector<ErrorReport> m_sub_errors;

	ErrorReport() = default;

	ErrorReport(std::string text)
	    : m_lines {{std::move(text)}} {}

	ErrorReport(std::string text, std::vector<ErrorReport> sub_errors)
	    : m_lines {{std::move(text)}}
	    , m_sub_errors {std::move(sub_errors)} {}

	static ErrorReport single(std::string);
	static ErrorReport single(std::string, std::vector<ErrorReport>);

	static ErrorReport multi(std::vector<std::string>);
	static ErrorReport multi(std::vector<std::string>, std::vector<ErrorReport>);

	bool ok() const;
	void print(int d = 1) const;

	void add_sub_error(ErrorReport);
};

ErrorReport make_located_error(string_view text, SourceLocation location);
