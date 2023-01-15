#include "error_messages.hpp"

#include "token.hpp"
#include "frontend_context.hpp"
#include "./utils/error_report.hpp"

#include <vector>
#include <sstream>
#include <string>

#include <cassert>

struct SourceLine { int start, end, number; };

static ErrorReport with_location(ErrorReport error, Frontend::Context const& file_context, int position);
static ErrorReport with_context(ErrorReport error, Frontend::Context const& file_context, int position, std::string message);

static std::vector<std::string> show_line_with_indicator(Frontend::Context const& file_context, int position, std::string message);
static SourceLine containing_line(Frontend::Context const& file_context, int position);
static bool is_closing_token(TokenTag tag);
static TokenTag matching_token(TokenTag tag);

ErrorReport make_unexpected_error(
    Frontend::Context const& file_context, std::string expected, Token const* found) {

	std::stringstream header;
	header << "Unexpected token: expected " << expected << " but found '" << found->m_text << "' instead";

	auto error = ErrorReport::single(header.str());
	error = with_location(std::move(error), file_context, found->m_start_offset);
	error = with_context(std::move(error), file_context, found->m_start_offset, "unexpected token is here");
	return error;
}

ErrorReport make_unexpected_error_with_open_brace(
    Frontend::Context const& file_context,
    Token const* opening,
    std::string expected,
    Token const* found
) {

	auto matching = matching_token(opening->m_type);

	if (found->m_type == TokenTag::END) {
		std::stringstream header;
		header << "Mismatched brace: found no '" << token_string[(int)matching] << "' to match '" << opening->m_text << "'";
		auto error = ErrorReport::single(header.str());
		error = with_location(std::move(error), file_context, opening->m_start_offset);
		error = with_context(std::move(error), file_context, opening->m_start_offset, "opening brace is here");
		return error;

	} else if (is_closing_token(found->m_type)) {
		std::stringstream header;
		header << "Mismatched braces: expected '" << token_string[(int)matching] << "' but found '" << found->m_text << "' instead";
		auto error = ErrorReport::single(header.str());
		error = with_location(std::move(error), file_context, found->m_start_offset);
		error = with_context(std::move(error), file_context, opening->m_start_offset, "opening brace is here");
		error = with_context(std::move(error), file_context, found->m_start_offset, "unexpected token is here");
		return error;

	} else {
		return make_unexpected_error(file_context, std::move(expected), found);
	}
}

ErrorReport make_located_error(
    Frontend::Context const& file_context,
    std::string text,
    Token const* token,
    std::string comment) {
	auto error = ErrorReport::single(text);
	error = with_location(std::move(error), file_context, token->m_start_offset);
	error = with_context(std::move(error), file_context, token->m_start_offset, std::move(comment));
	return error;
}

static ErrorReport with_location(ErrorReport error, Frontend::Context const& file_context, int position) {
	SourceLocation location = file_context.char_offset_to_location(position);
	std::stringstream prefix;
	prefix << "At " << location.to_string() << ": ";
	error.m_lines[0] = prefix.str() + error.m_lines[0];
	return error;
}

static ErrorReport with_context(ErrorReport error, Frontend::Context const& file_context, int position, std::string message) {
	auto code_display = show_line_with_indicator(file_context, position, std::move(message));
	error.m_lines.push_back(std::move(code_display[0]));
	error.m_lines.push_back(std::move(code_display[1]));
	return error;
}

static std::vector<std::string> show_line_with_indicator(Frontend::Context const& file_context, int position, std::string message) {

	std::stringstream content;
	auto line = containing_line(file_context, position);
	auto line_content = file_context.source.substr(line.start, line.end - line.start);
	auto normalized_line_content = std::string();
	for (char c : line_content) {
		if (c == '\t') {
			normalized_line_content.push_back(' ');
			normalized_line_content.push_back(' ');
			normalized_line_content.push_back(' ');
			normalized_line_content.push_back(' ');
		} else {
			normalized_line_content.push_back(c);
		}
	}
	auto line_number = std::to_string(line.number);
	content << line_number << " | " << normalized_line_content;

	std::stringstream footer;
	int byte_position = int(line_number.size()) + 3 + (position - line.start);
	int column = 0;
	for (int i = 0; i < byte_position; ++i) {
		column += line_content[i] == '\t' ? 4 : 1;
	}

	footer << std::string(column, ' ') << "^ " << message;
	auto footer_str = footer.str();
	footer_str[line_number.size() + 1] = '|';

	return {{content.str(), footer_str}};
}

static SourceLine containing_line(Frontend::Context const& file_context, int position) {
	int line_start = 0;
	for (int i = position; i > 0; --i) {
		if (file_context.source[i-1] == '\n') {
			line_start = i;
			break;
		}
	}

	int line_end = int(file_context.source.size());
	for (int i = position; i < int(file_context.source.size()); ++i) {
		if (file_context.source[i] == '\n') {
			line_end = i;
			break;
		}
	}

	int line_number = 1;
	for (int i = 0; i < line_start; ++i) {
		if (file_context.source[i] == '\n') {
			line_number += 1;
		}
	}

	return {line_start, line_end, line_number};
}

static bool is_closing_token(TokenTag tag) {
	switch (tag) {
	case TokenTag::BRACE_CLOSE:
	case TokenTag::BRACKET_CLOSE:
	case TokenTag::PAREN_CLOSE:
	case TokenTag::POLY_CLOSE:
		return true;
	default: return false;
	}
}

static TokenTag matching_token(TokenTag tag) {
	switch (tag) {
	case TokenTag::BRACE_OPEN: return TokenTag::BRACE_CLOSE;
	case TokenTag::BRACKET_OPEN: return TokenTag::BRACKET_CLOSE;
	case TokenTag::PAREN_OPEN: return TokenTag::PAREN_CLOSE;
	case TokenTag::POLY_OPEN: return TokenTag::POLY_CLOSE;
	default: assert(0);
	}
}
