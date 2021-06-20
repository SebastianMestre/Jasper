#pragma once

#include "log/log.hpp"

#include "nodes.hpp"
#include "relationships.hpp"

#include <vector>
#include <algorithm>

#include <cassert>

// INVARIANT3: if (A IsDotResultOf B && A IsDotResultOf C) then B = C

// INVARIANT1: relationships never point at a non-singleton variable
// INVARIANT2: there are no cycles between variables
struct Engine {
	Nodes nodes;
	Relationships relationships;

	void unify(int idx1, int idx2) {
		idx1 = nodes.find(idx1);
		idx2 = nodes.find(idx2);

		// TODO: occurs check ?
		relationships.transfer(idx1, idx2);
		nodes.point_at(idx1, idx2);
	}

	bool prove_error(int idx) {
		// TODO: implement the following rules

		// error(X) :- is_dot_result_of(X, Y), func(X).
		// error(X) :- is_dot_result_of(Y, X), func(X).
		// error(X) :- is_dot_result_of(Y, X), ctor(X).
		// error(X) :- is_dot_result_of(X, Y), mono(X).

		// error(X) :- mono(X), term(X)
		// error(X) :- mono(X), func(X)
		// error(X) :- mono(X), ctor(X)
		// error(X) :- term(X), func(X)
		// error(X) :- term(X), ctor(X)
		// error(X) :- func(X), ctor(X)
		return false;
	}

	void infer(int X) {
		// TODO: unlike a typical prolog implementation, this implementation
		// does no backtracking or recursion. It might actually need to.

		auto is_term = [&](int idx) -> bool {
			return relationships.exists(Relationship::Tag::IsTerm, 0, idx);
		};

		if (!is_term(X)) {

			bool should_be_term = false;

			// term(X) :- is_dot_result_of(X, Y), term(Y).
			// X is dot result of Y && Y is term => X is term
			std::vector<int> dot_targets =
			    relationships.gather(Relationship::Tag::IsDotResultOf, 0, 1, X);
			for (int Y : dot_targets) {
				if (should_be_term) break;
				if (is_term(Y))
					should_be_term = true;
			}

			// term(X) :- is_dot_result_of(Y, X), term(Y).
			// Y is dot result of X && Y is term => X is term
			std::vector<int> dot_results =
			    relationships.gather(Relationship::Tag::IsDotResultOf, 1, 0, X);
			for (int Y : dot_results) {
				if (should_be_term) break;
				if (is_term(Y))
					should_be_term = true;
			}

			if (should_be_term)
				relationships.create(Relationship::Tag::IsTerm, X);
		}

		// TODO: implement the following rules
		// ctor(X) :- is_dot_result_of(X, Y), mono(Y).
		// mono(Y) :- is_dot_result_of(X, Y), ctor(X).
	}

	int eval(int idx) {
		idx = nodes.find(idx);

		infer(idx);

		if (prove_error(idx)) {
			// TODO: more error checking
			Log::fatal() << "contradiction";
		}

		return idx;
	}

	int create_var() {
		return nodes.create_node();
	}

	void is_dot_result(int idx1, int idx2) {
		// find the variables first, so that we uphold INVARIANT1
		idx1 = nodes.find(idx1);
		idx2 = nodes.find(idx2);
		relationships.create(Relationship::Tag::IsDotResultOf, idx1, idx2);
	}
};
