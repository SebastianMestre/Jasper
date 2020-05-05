#pragma once

#include "value.hpp"

namespace Type {

struct Error : Value {
	std::string m_error;

	Error() = default;
	Error(std::string);

	void gc_visit() override;

};

Error make_reference_error(const Identifier&);
Error make_range_error(int,int);

}
