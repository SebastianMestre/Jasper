#pragma once

#include <string>
#include <type_traits>

#include "error_report.hpp"
#include "token_array.hpp"

namespace CST {
struct CST;
struct Allocator;
}

template <typename T>
struct Writer {
	Writer() = default;
	Writer(Writer const&) = default;
	Writer(Writer&&) = default;

	Writer(ErrorReport error)
	    : m_error {std::move(error)} {}

	Writer(ErrorReport error, T const& result)
	    : m_error {std::move(error)}
	    , m_result {result} {}

	Writer(ErrorReport error, T&& result)
	    : m_error {std::move(error)}
	    , m_result {std::move(result)} {}

	Writer& operator=(Writer const&) = default;
	Writer& operator=(Writer&&) = default;

	template <typename U>
	Writer(Writer<U>&& o)
	    : m_error {std::move(o.m_error)}
	    , m_result {std::move(o.m_result)} {}

	ErrorReport m_error {};
	T m_result {};

	bool ok() {
		return m_error.ok();
	}
};

Writer<CST::CST*> parse_program(TokenArray const&, CST::Allocator&);
Writer<CST::CST*> parse_expression(TokenArray const&, CST::Allocator&);
