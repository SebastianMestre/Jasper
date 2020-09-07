#include "error_report.hpp"

#include <iostream>

bool ErrorReport::ok() {
	return m_sub_errors.empty() && m_text.empty();
}

void ErrorReport::print(int d) {
	for (int i = 0; i < d; ++i)
		std::cerr << '-';

	std::cerr << ' ' << m_text << '\n';

	for (auto& sub : m_sub_errors)
		sub.print(d + 1);
}
