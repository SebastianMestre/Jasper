#pragma once

#include <string>

#include "error_report.hpp"
#include "token_array.hpp"

namespace CST {
struct CST;
struct Allocator;
}

template <typename T>
struct Writer {
	ErrorReport m_error {};
	T m_result {};

	bool ok() {
		return m_error.ok();
	}
};

TokenArray tokenize(std::string const& source);
Writer<CST::CST*> parse_program(TokenArray&, CST::Allocator&);
Writer<CST::CST*> parse_expression(TokenArray&, CST::Allocator&);
