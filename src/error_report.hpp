#pragma once

#include <string>
#include <vector>

struct ErrorReport {
	std::string m_text;
	std::vector<ErrorReport> m_sub_errors;

	bool ok();
	void print(int d = 1);
};
