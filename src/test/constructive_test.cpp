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
	letterA,
	letterB,
	number1,
	number2,
	open_brackets,
	close_brackets,
	return_true
};

const std::set<Simbol> terminalSimbols {
	fn, assign, invoke, paren, end,
    letterA, letterB,
    number1, number2,
    open_brackets, close_brackets,
	return_true
};

const std::unordered_map<Simbol, std::string> finalReplacement {
    {fn, "fn"},
    {assign, " := "},
    {invoke, "__invoke"},
    {paren, "()"},
    {end, ";"},
    {letterA, "a"},
    {letterB, "b"},
    {number1, "1"},
    {number2, "2"},
    {open_brackets, "{"},
    {close_brackets, "}"},
    {return_true, "return true"}
};

const std::unordered_map<Simbol, std::vector<std::vector<Simbol>>> intermidiateReplacement {
    {SIGMA, {{DECLARATION, INVOKE}}},
    {INVOKE, {{invoke, assign, fn, paren, open_brackets, return_true, end, close_brackets, end}}},//Always return true
	{BODY, {{}}},
    {DECLARATION,
        {{end}, {VAR_NAME, assign, NUMBER, end}, {DECLARATION, DECLARATION}}},
    {VAR_NAME, {{letterA}, {letterB}, {VAR_NAME, VAR_NAME}}},
    {NUMBER, {{number1}, {number2}, {NUMBER, NUMBER}}}
};

const std::unordered_map<Simbol, std::discrete_distribution<int>> intermidiateReplacementDist {
    {SIGMA, {1}},
    {INVOKE, {1}},
	{BODY, {1}},
    {DECLARATION, {0.5, 1, 0.5}},
    {VAR_NAME, {1, 1, 1}},
    {NUMBER, {1, 1, 0.5}}};

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

	for (auto v : list) {
		assert(terminalSimbols.find(v) != terminalSimbols.end());
		finalExpression += finalReplacement.at(v);
	}

	return finalExpression;
}

}