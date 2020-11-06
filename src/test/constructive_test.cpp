#include "constructive_test.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <set>
#include <random>
#include <assert.h>

namespace Test {

enum Simbol {
	SIGMA,
	DECLARATION,
	INVOKE,
	VAR_NAME,
	NUMBER,
	BODY,
	fn,
	assign,
	invoke,
	paren,
	end,
	unrepeatable_name,
	number,
	paren_open, paren_close,
	open_brackets, close_brackets,
	return_val,
	true_val, false_val
};

const std::set<Simbol> terminalSimbols {
	fn, assign, invoke, end,
	unrepeatable_name,
    number,
	paren_open, paren_close,
    open_brackets, close_brackets,
	return_val,
	true_val, false_val
};

const std::unordered_map<Simbol, std::string> finalReplacement {
    {fn, "fn"},
    {assign, " := "},
    {invoke, "__invoke"},
    {paren_open, "("},
    {paren_close, ")"},
    {end, ";"},
    {open_brackets, "{"},
    {close_brackets, "}"},
    {return_val, "return "},
    {true_val, "true"},
    {false_val, "false"}
};

const std::unordered_map<Simbol, std::vector<std::vector<Simbol>>> intermidiateReplacement {
    {SIGMA, {{DECLARATION, INVOKE}}},
    {INVOKE, {{invoke, assign, fn, paren_open, paren_close, open_brackets, BODY, return_val, true_val, end, close_brackets, end}}},//Always return true
	{BODY, {{DECLARATION}}},
    {DECLARATION,
        {{VAR_NAME, assign, NUMBER, end}, {DECLARATION, DECLARATION}}},
    {VAR_NAME, {{unrepeatable_name}}},
    {NUMBER, {{number}}}
};

const std::unordered_map<Simbol, std::discrete_distribution<int>> intermidiateReplacementDist {
    {SIGMA, {1}},
    {INVOKE, {1}},
	{BODY, {1}},
    {DECLARATION, {1, 0.5}},
    {VAR_NAME, {1}},
    {NUMBER, {1}}};

const std::vector<char> allowedChars = {'a', 'b'};

std::string generateRandomName(std::mt19937& gen) {
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

std::string replacement(std::mt19937& rg, std::set<std::string>& usedNames, const Simbol simbol) {
	switch (simbol) {
	case number:
		return "12";
	case unrepeatable_name: {
		std::string name;

		do {
			name = generateRandomName(rg);
		} while (usedNames.find(name) != usedNames.end());

		usedNames.emplace(name);

		return name;
	}
	default:
		assert(terminalSimbols.find(simbol) != terminalSimbols.end());
		return finalReplacement.at(simbol);
	}
}

std::string generate() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::list<Simbol> list;
	list.push_back(SIGMA);

	uint terminalsInList = 0;

	auto i = list.begin();
	while (terminalsInList < list.size()) {
		if (i == list.begin()) {
			terminalsInList = 0;
		}

		bool terminal = terminalSimbols.find(*i) != terminalSimbols.end();

		if (!terminal) {
			auto replacementDistribution = intermidiateReplacementDist.at(*i);
			std::vector<std::vector<Simbol>> currentReplacementPossibilities =
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

	std::string finalExpression = "";
	std::set<std::string> usedNames;

	for (auto v : list) {
		finalExpression += replacement(gen, usedNames, v);
	}

	return finalExpression;
}

}