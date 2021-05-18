#pragma once

#define TEST_STATUS                                                            \
	X(Ok)                                                                      \
	X(Error)                                                                   \
	X(Fail)                                                                    \
	X(Empty)                                                                   \
	X(MissingFile)

#define X(name) #name,
constexpr const char* test_status_string[] = {TEST_STATUS};
#undef X

#define X(name) name,
enum class TestStatus { TEST_STATUS };
#undef X

#undef TEST_STATUS

struct TestReport {
	TestStatus m_code;
	std::string m_msg;
};
