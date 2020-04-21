#include "error.hpp"

#include <sstream>

namespace Type {

Error::Error(std::string message) : m_error(message) {}

void Error::gc_visit() {
	Value::m_visited = true;
}

ReferenceError::ReferenceError(Identifier i) {
	std::stringstream ss;
	ss << "ReferenceError: '" << i << "' is not a valid identifier";

	Error(ss.str());
}

RangeError::RangeError(int accessed, int actual_size) {
	std::stringstream ss;
	ss
		<< "RangeError: tried to access position " << accessed
		<< " of list of size " << actual_size;

	Error(ss.str());
}

}
