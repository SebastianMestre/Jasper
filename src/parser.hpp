#pragma once

#include <string>

#include "./utils/writer.hpp"
#include "token_array.hpp"

namespace CST {
struct CST;
struct Allocator;
}

namespace Frontend {
struct Context;
}

Writer<CST::CST*> parse_program(TokenArray const&, Frontend::Context const&, CST::Allocator&);
Writer<CST::CST*> parse_expression(TokenArray const&, Frontend::Context const&, CST::Allocator&);
