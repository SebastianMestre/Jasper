#pragma once

#include <string>
#include <vector>

#include "test_status_tag.hpp"

struct TestReportPiece {
	TestStatus m_status;
	std::string m_message;
};

struct TestReport {
	std::vector<TestReportPiece> m_pieces;

	TestReport() = default;

	TestReport(TestStatus status)
	    : m_pieces {{status, ""}} {}

	TestReport(TestStatus status, std::string message)
	    : m_pieces {{status, std::move(message)}} {}

	TestReport(TestReportPiece piece)
	    : m_pieces {{std::move(piece)}} {}

	TestReport& operator += (TestReport const& o) {
		m_pieces.insert(m_pieces.end(), o.m_pieces.begin(), o.m_pieces.end());
		return *this;
	};

	TestReport operator + (TestReport const& o) const {
		auto copy = *this;
		copy += o;
		return copy;
	}

	void print();
	TestStatus status();
};
