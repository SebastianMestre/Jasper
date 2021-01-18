#include <string>
#include <iosfwd>
#include "../utils/interned_string.hpp"

namespace Log {

struct LogStream {
	LogStream& operator<<(InternedString const&);
	LogStream& operator<<(std::string const&);
	LogStream& operator<<(char const*);
	LogStream& operator<<(int);
	LogStream& operator<<(float);
	LogStream& operator<<(char);

	~LogStream();
protected:
	LogStream(std::ostream&);
private:
	std::ostream& m_stream;

};

} // namespace Log
