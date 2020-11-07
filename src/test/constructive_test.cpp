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

static std::set<Symbol> const terminalSymbols {
	Symbol::fn, Symbol::assign, Symbol::invoke, Symbol::end,
	Symbol::unrepeatable_name,
	Symbol::number,
	Symbol::paren_open, Symbol::paren_close,
	Symbol::brackets_open, Symbol::brackets_close,
	Symbol::return_val,
	Symbol::true_val, Symbol::false_val
};

static std::unordered_map<Symbol, std::string> const finalReplacement {
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

static std::unordered_map<Symbol, std::vector<std::vector<Symbol>>> const intermidiateReplacement {
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

static std::unordered_map<Symbol, std::discrete_distribution<int>> const intermidiateReplacementDist {
    {Symbol::SIGMA, {1}},
    {Symbol::INVOKE, {1}},
	{Symbol::BODY, {1}},
    {Symbol::DECLARATION, {1, 0.5}},
    {Symbol::VAR_NAME, {1}},
    {Symbol::NUMBER, {1}}};

static std::vector<char> const allowedChars = {'a', 'b'};

static std::string generateRandomName(std::mt19937& gen) {
	static std::uniform_int_distribution<int> nameLengthDistribution(1, 25);
	static std::uniform_int_distribution<int> characterPositionDistribution(0, allowedChars.size()-1);

	int length = nameLengthDistribution(gen);

	std::string s;
	s.reserve(length);

	for (int i = 0; i < length; ++i) {
		s += allowedChars[characterPositionDistribution(gen)];
	}

	return s;
}

static std::string replacement(std::mt19937& rg, std::set<std::string>& usedNames, Symbol const symbol) {
	switch (symbol) {
	case Symbol::number:
		return "12";
	case Symbol::unrepeatable_name: {
		std::string name;

		do {
			name = generateRandomName(rg);
		} while (usedNames.find(name) != usedNames.end());

		usedNames.emplace(name);

		return name;
	}
	default:
		assert(terminalSymbols.find(symbol) != terminalSymbols.end());
		return finalReplacement.at(symbol);
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

		bool terminal = terminalSymbols.find(*i) != terminalSymbols.end();

		if (!terminal) {
			auto replacementDistribution = intermidiateReplacementDist.at(*i);
			std::vector<std::vector<Symbol>> currentReplacementPossibilities =
			    intermidiateReplacement.at(*i);
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

	std::ostringstream finalExpression;
	std::set<std::string> usedNames;

	for (auto v : list) {
		finalExpression << replacement(gen, usedNames, v);
	}

	return finalExpression.str();
}

}