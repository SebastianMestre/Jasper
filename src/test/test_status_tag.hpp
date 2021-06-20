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

inline TestStatus worst_of(TestStatus lhs, TestStatus rhs) {
	if (lhs == TestStatus::Error || rhs == TestStatus::Error)
		return TestStatus::Error;
	if (lhs == TestStatus::Fail || rhs == TestStatus::Fail)
		return TestStatus::Fail;
	if (lhs == TestStatus::Empty || rhs == TestStatus::Empty)
		return TestStatus::Empty;
	if (lhs == TestStatus::MissingFile || rhs == TestStatus::MissingFile)
		return TestStatus::MissingFile;
	return TestStatus::Ok;
}

struct TestReport {
	TestStatus m_code;
	std::string m_msg;
};
