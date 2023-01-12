#include "error_report.hpp"

#include <iostream>
#include <sstream>

constexpr char const* marker = "--";
constexpr char const* spacer = "  ";

ErrorReport ErrorReport::single(std::string text) {
	ErrorReport result;
	result.m_lines = {{std::move(text)}};
	return result;
}

ErrorReport ErrorReport::single(std::string text, std::vector<ErrorReport> sub_errors) {
	ErrorReport result;
	result.m_lines = {{std::move(text)}};
	result.m_sub_errors = std::move(sub_errors);
	return result;
}

ErrorReport ErrorReport::multi(std::vector<std::string> lines) {
	ErrorReport result;
	result.m_lines = std::move(lines);
	return result;
}

ErrorReport ErrorReport::multi(std::vector<std::string> lines, std::vector<ErrorReport> sub_errors) {
	ErrorReport result;
	result.m_lines = std::move(lines);
	result.m_sub_errors = std::move(sub_errors);
	return result;
}


bool ErrorReport::ok() const {
	return m_sub_errors.empty() && m_lines.empty();
}


void ErrorReport::print(int d) const {

	bool first_line = true;

	for (auto const& line : m_lines) {
		for (int i = 0; i < d; ++i)
			std::cerr << (first_line ? marker : spacer);

		std::cerr << ' ' << line << '\n';

		first_line = false;
	}

	for (auto& sub : m_sub_errors)
		sub.print(d + 1);
}

void ErrorReport::add_sub_error(ErrorReport other) {
	m_sub_errors.push_back(std::move(other));
}


ErrorReport make_located_error(string_view text, SourceLocation location) {
	std::stringstream ss;
	ss << "At " << location.to_string() << " : " << text;
	return ErrorReport {ss.str()};
}
