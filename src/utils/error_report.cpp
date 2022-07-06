#include "error_report.hpp"

#include <iostream>
#include <sstream>

bool ErrorReport::ok() const {
	return m_sub_errors.empty() && m_text.empty();
}

void ErrorReport::print(int d) const {
	for (int i = 0; i < d; ++i)
		std::cerr << '-';

	std::cerr << ' ' << m_text << '\n';

	for (auto& sub : m_sub_errors)
		sub.print(d + 1);
}

ErrorReport make_located_error(string_view text, SourceLocation location) {
	std::stringstream ss;
	ss << "At " << location.to_string() << " : " << text;
	return ErrorReport {ss.str()};
}
