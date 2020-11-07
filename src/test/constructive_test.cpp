#include "constructive_test.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <unordered_map>
#include <set>
#include <random>
#include <assert.h>

namespace Test {

enum class Symbol {
	SIGMA,
	DECLARATION,
	INVOKE,
	VAR_NAME,
	NUMBER,
	BODY,
	fn,
	assign,
	invoke,
	end,
	unrepeatable_name,
	number,
	paren_open, paren_close,
	brackets_open, brackets_close,
	return_val,
	true_val, false_val
};

static std::set<Symbol> const terminal_symbols {
	Symbol::fn, Symbol::assign, Symbol::invoke, Symbol::end,
	Symbol::unrepeatable_name,
	Symbol::number,
	Symbol::paren_open, Symbol::paren_close,
	Symbol::brackets_open, Symbol::brackets_close,
	Symbol::return_val,
	Symbol::true_val, Symbol::false_val
};

static std::unordered_map<Symbol, std::string> const final_replacement {
    {Symbol::fn, "fn"},
    {Symbol::assign, " := "},
    {Symbol::invoke, "__invoke"},
    {Symbol::paren_open, "("},
    {Symbol::paren_close, ")"},
    {Symbol::end, ";"},
    {Symbol::brackets_open, "{"},
    {Symbol::brackets_close, "}"},
    {Symbol::return_val, "return "},
    {Symbol::true_val, "true"},
    {Symbol::false_val, "false"}
};

static std::unordered_map<Symbol, std::vector<std::vector<Symbol>>> const intermidiate_replacement {
    {Symbol::SIGMA, {{Symbol::DECLARATION, Symbol::INVOKE}}},
    {Symbol::INVOKE,
     {{
         Symbol::invoke, Symbol::assign, Symbol::fn, Symbol::paren_open,
         Symbol::paren_close, Symbol::brackets_open, Symbol::BODY,
         Symbol::return_val,Symbol::true_val, Symbol::end, Symbol::brackets_close, Symbol::end //Always return true
         }}
    },
	{Symbol::BODY, {{Symbol::DECLARATION}}},
    {Symbol::DECLARATION,
     {
         {Symbol::VAR_NAME, Symbol::assign, Symbol::NUMBER, Symbol::end},
         {Symbol::DECLARATION, Symbol::DECLARATION}
     }},
    {Symbol::VAR_NAME, {{Symbol::unrepeatable_name}}},
    {Symbol::NUMBER, {{Symbol::number}}}
};

static std::unordered_map<Symbol, std::discrete_distribution<int>> const intermidiate_replacementDist {
    {Symbol::SIGMA, {1}},
    {Symbol::INVOKE, {1}},
	{Symbol::BODY, {1}},
    {Symbol::DECLARATION, {1, 0.5}},
    {Symbol::VAR_NAME, {1}},
    {Symbol::NUMBER, {1}}};

static std::vector<char> const allowed_chars = {'a', 'b'};

static std::string generate_random_name(std::mt19937& gen) {
	static std::uniform_int_distribution<int> name_len_dist(1, 25);
	static std::uniform_int_distribution<int> char_pos_dist(0, allowed_chars.size()-1);

	int length = name_len_dist(gen);

	std::string name;
	name.reserve(length);

	for (int i = 0; i < length; ++i) {
		name += allowed_chars[char_pos_dist(gen)];
	}

	return name;
}

static std::string replacement(std::mt19937& rg,
    std::set<std::string>& used_names, Symbol const symbol) {
	switch (symbol) {
	case Symbol::number:
		return "12";
	case Symbol::unrepeatable_name: {
		std::string name;

		do {
			name = generate_random_name(rg);
		} while (used_names.find(name) != used_names.end());

		used_names.emplace(name);

		return name;
	}
	default:
		assert(terminal_symbols.find(symbol) != terminal_symbols.end());
		return final_replacement.at(symbol);
	}
}

std::string generate() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::list<Symbol> list;
	list.push_back(Symbol::SIGMA);

	uint terminalsInList = 0;

	auto i = list.begin();
	while (terminalsInList < list.size()) {
		if (i == list.begin()) {
			terminalsInList = 0;
		}

		bool terminal = terminal_symbols.find(*i) != terminal_symbols.end();

		if (!terminal) {
			auto replacementDistribution = intermidiate_replacementDist.at(*i);
			std::vector<std::vector<Symbol>> currentReplacementPossibilities =
			    intermidiate_replacement.at(*i);
			assert(
			    currentReplacementPossibilities.size() ==
			    replacementDistribution.probabilities().size());

			auto replacement =
			    currentReplacementPossibilities[replacementDistribution(rd)];

			list.insert(i, replacement.begin(), replacement.end());
			i = list.erase(i);
		} else {
			terminalsInList++;
			++i;
		}

		if (i == list.end()) {
			i = list.begin();
		}
	}

	std::ostringstream final_expression;
	std::set<std::string> used_names;

	for (auto v : list) {
		final_expression << replacement(gen, used_names, v);
	}

	return final_expression.str();
}

}