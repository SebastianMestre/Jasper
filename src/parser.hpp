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

	Writer() = default;

	Writer(Writer const&) = default;
	Writer(Writer&&) = default;

	Writer& operator=(Writer const&) = default;
	Writer& operator=(Writer&&) = default;

	Writer(ErrorReport error)
	    : m_error {std::move(error)} {}

	Writer(ErrorReport error, T result)
	    : m_error {std::move(error)}
	    , m_result {std::move(result)} {}

	void add_sub_error(ErrorReport err) {
		m_error.m_sub_errors.push_back(std::move(err));
	}

	ErrorReport& error() & {
		return m_error;
	}
	ErrorReport const& error() const& {
		return m_error;
	}
	ErrorReport&& error() && {
		return std::move(m_error);
	}

	bool ok() {
		return m_error.ok();
	}
};

Writer<CST::CST*> parse_program(TokenArray const&, CST::Allocator&);
Writer<CST::CST*> parse_expression(TokenArray const&, CST::Allocator&);
