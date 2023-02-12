#pragma once

#include "frontend_context.hpp"

#include <vector>

struct Token;

struct LexerResult {
	Frontend::Context file_context;
	std::vector<Token> tokens;
};
