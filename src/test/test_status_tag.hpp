#pragma once

constexpr const char* test_status_string[] = {
	"Ok",
	"Error",
	"Fail",
	"Empty",
};

enum class TestStatusTag {
	Ok = 0,
	Error,
	Fail,
	Empty,
};

struct TestReport {
	TestStatusTag m_code;
	std::string m_msg;
};
