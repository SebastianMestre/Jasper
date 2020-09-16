#pragma once

#define TEST_STATUS_TAGS \
	X(Ok) \
	X(Error) \
	X(Fail) \
	X(Empty)

#define X(name) "name",
constexpr const char* test_status_string[] = {
	TEST_STATUS_TAGS
};
#undef X

#define X(name) name,
enum class TestStatusTag {
	TEST_STATUS_TAGS
};
#undef X

#undef TEST_STATUS_TAGS

struct TestReport {
	TestStatusTag m_code;
	std::string m_msg;
};
