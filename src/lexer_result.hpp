#pragma once

#include "token_array.hpp"
#include "frontend_context.hpp"

struct LexerResult {
	Frontend::Context file_context;
	TokenArray tokens;
};
