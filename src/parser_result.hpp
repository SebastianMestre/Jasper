#pragma once

#include "./utils/error_report.hpp"
#include "frontend_context.hpp"
#include "token_array.hpp"

#include <cassert>

namespace CST {
struct CST;
}

struct ParserResult {
	ParserResult(CST::CST* cst, ErrorReport error, TokenArray tokens, Frontend::Context file_context)
	    : m_cst {cst}
		, m_file_context {std::move(file_context)}
	    , m_error {std::move(error)}
		, m_tokens {std::move(tokens)} {}

	bool ok() const {
		return m_error.ok();
	}

	CST::CST* cst() const {
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
	CST::CST* m_cst;
	Frontend::Context m_file_context;
	ErrorReport m_error;
	TokenArray m_tokens;
};
