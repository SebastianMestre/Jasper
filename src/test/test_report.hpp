#pragma once

#include <numeric>
#include <string>
#include <vector>
#include <iostream>

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


	TestReport& operator += (TestReport const& o) {
		m_pieces.insert(m_pieces.end(), o.m_pieces.begin(), o.m_pieces.end());
		return *this;
	};

	TestReport operator + (TestReport const& o) const {
		auto copy = *this;
		copy += o;
		return copy;
	}

	void print() {
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

	TestStatus status() {
		return std::accumulate(
		    m_pieces.begin(),
		    m_pieces.end(),
		    TestStatus::Ok,
		    [](TestStatus status, TestReportPiece const& piece) {
			    return worst_of(status, piece.m_status);
		    });
	}
};
