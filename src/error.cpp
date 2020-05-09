#include "error.hpp"

#include <sstream>

namespace Type {

Error::Error() : Value(value_type::Error) {}
Error::Error(std::string message) : Value(value_type::Error), m_error(message) {}

void Error::gc_visit() {
	Value::m_visited = true;
}

Error make_reference_error(const Identifier& i) {
	std::stringstream ss;
	ss << "ReferenceError: '" << i << "' is not a valid identifier";

	return Error(ss.str());
}

Error make_range_error(int accessed, int actual_size) {
	std::stringstream ss;
	ss
		<< "RangeError: tried to access position " << accessed
		<< " of list of size " << actual_size;

	return Error(ss.str());
}

}
