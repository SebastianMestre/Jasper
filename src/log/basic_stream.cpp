#include "basic_stream.hpp"

#include <ostream>

namespace Log {

LogStream::LogStream(std::ostream& stream)
    : m_stream(stream) {}

LogStream::~LogStream() {
	m_stream << "\n";
}

#define INSTANCE_SHIFT_OPERATOR(type)                                          \
	LogStream& LogStream::operator<<(type in) {                                \
		m_stream << in;                                                        \
		return *this;                                                          \
	}

INSTANCE_SHIFT_OPERATOR(InternedString const&)
INSTANCE_SHIFT_OPERATOR(std::string const&)
INSTANCE_SHIFT_OPERATOR(char const*)
INSTANCE_SHIFT_OPERATOR(int)
INSTANCE_SHIFT_OPERATOR(float)
INSTANCE_SHIFT_OPERATOR(char)

} // namespace Log
