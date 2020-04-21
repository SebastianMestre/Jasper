#pragma once

#include "runtime.hpp"

namespace Type {

struct Error : Value {
	std::string m_error;

	Error() = default;
	Error(std::string);

	void gc_visit() override;
};

struct ReferenceError : Error {

	ReferenceError(Identifier);
};

struct RangeError : Error {

	RangeError(int, int);
};

}
