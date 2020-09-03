#pragma once

constexpr const char* test_status_string[] = {
    "Ok",
    "Error",
    "Fail",
    "Empty",
};

enum class test_status {
	Ok = 0,
	Error,
	Fail,
	Empty,
};

struct TestReport {
	test_status m_code;
	std::string m_msg;
};
