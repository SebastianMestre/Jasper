#pragma once

#include <ostream>
#include <string>

#include "./cst.hpp"

namespace CST {

void pretty_print(const CST*, std::ostream&);

std::string pretty_print(const CST*);

} // namespace CST
