#pragma once

#include "./utils/error_report.hpp"
#include "frontend_context.hpp"
#include "token_array.hpp"

#include <cassert>

template<typename T>
struct ParserResult {
	// TODO: check that T is CST or a subtype of CST

	ParserResult(T* cst, ErrorReport error, TokenArray tokens, Frontend::Context file_context)
	    : m_cst {cst}
		, m_file_context {std::move(file_context)}
	    , m_error {std::move(error)}
		, m_tokens {std::move(tokens)} {}

	bool ok() const {
		return m_error.ok();
	}

	T* cst() const {
		assert(ok());
		return m_cst;
	}

	Frontend::Context const& file_context() const {
		return m_file_context;
	}

	ErrorReport const& error() const {
		return m_error;
	}

private:
	T* m_cst;
	Frontend::Context m_file_context;
	ErrorReport m_error;
	TokenArray m_tokens;
};
