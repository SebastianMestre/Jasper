#pragma once

#include <vector>
#include <array>

struct Relationship {
	enum class Tag { IsTerm, IsMono, IsFunc, IsCtor, IsDotResultOf };

	Tag tag;
	std::array<int, 2> args;

	// IsDotResultOf:
	//  0: metatype of "A.B"
	//  1: metatype of "A"
};

bool operator== (Relationship const& lhs, Relationship const& rhs) {
	return lhs.tag == rhs.tag && lhs.args == rhs.args;
}
bool operator!= (Relationship const& lhs, Relationship const& rhs) {
	return !(lhs == rhs);
}

struct Relationships {
	std::vector<Relationship> relationships;

	int create(Relationship::Tag t, int idx) {
		return create(t, idx, -1);
	}

	int create(Relationship::Tag t, int idx1, int idx2) {
		int result = relationships.size();
		relationships.push_back({t, {idx1, idx2}});
		return result;
	}

	void transfer(int idx1, int idx2) {
		for (auto& relationship : relationships) {
			for (auto& arg : relationship.args) {
				if (arg == idx1) {
					arg = idx2;
				}
			}
		}
		// TODO: deduplicate relationships
	}

	std::vector<int> gather(
	    Relationship::Tag t, int comparison_arg, int desired_arg, int idx) const {
		// TODO: obviously, iterating over all relationships will be
		// extremely slow for larger programs. Use a better data structure
		std::vector<int> result;
		for (auto const& rel : relationships) {
			if (rel.tag == t && rel.args[comparison_arg] == idx) {
				result.push_back(rel.args[desired_arg]);
			}
		}
		return result;
	}

	bool exists(Relationship::Tag t, int comparison_arg, int idx) {
		// TODO: obviously, iterating over all relationships will be
		// extremely slow for larger programs. Use a better data structure
		for (auto const& rel : relationships) {
			if (rel.tag == t && rel.args[comparison_arg] == idx) {
				return true;
			}
		}
		return false;
	}
};

