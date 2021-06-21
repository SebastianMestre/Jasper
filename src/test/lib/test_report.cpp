#include "test_report.hpp"

#include <numeric>
#include <iostream>

void TestReport::print() {
	auto to_text = [](TestStatus status) -> char const* {
		switch (status) {
		case TestStatus::Ok:
			return ".";
		case TestStatus::Error:
			return "E";
		case TestStatus::Fail:
			return "F";
		case TestStatus::Empty:
			return "R";
		case TestStatus::MissingFile:
			return "?";
		default:
			return nullptr;
		}
	};

	for (auto const& piece : m_pieces) {
		auto text = to_text(piece.m_status);
		if (text) {
			std::cout << text;
		} else {
			std::cerr << "invalid test status";
		}
	}
	std::cout << '\n';

	for (auto const& piece : m_pieces) {
		std::cout << piece.m_message;
		if (!piece.m_message.empty())
			std::cout << '\n';
	}
}

TestStatus TestReport::compute_status() {
	return std::accumulate(
	    m_pieces.begin(),
	    m_pieces.end(),
	    TestStatus::Ok,
	    [](TestStatus status, TestReportPiece const& piece) {
		    return worst_of(status, piece.m_status);
	    });
}
