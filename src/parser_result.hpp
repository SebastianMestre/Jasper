#pragma once

#include "./utils/error_report.hpp"

#include <cassert>

namespace CST {
struct CST;
}

struct ParserResult {
	ParserResult(CST::CST* cst, ErrorReport error)
	    : m_cst {cst}
	    , m_error {std::move(error)} {}

	bool ok() const {
		return m_error.ok();
	}

	CST::CST* cst() const {
		assert(ok());
		return m_cst;
	}

	ErrorReport const& error() const {
		return m_error;
	}

private:
	CST::CST* m_cst;
	ErrorReport m_error;
};
