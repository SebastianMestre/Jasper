#pragma once

#include <vector>

namespace Unification {

enum class Tag { Var, Data, };

struct Header {
	Tag tag;
	int equals;
};

// NOTE: could potentially add <typename Id = int> if needed
template <typename T = int, typename U = Header>
struct Core {
	std::vector<U> header;
	std::vector<T> data;

	Core() = default;

	virtual int new_var() = 0;

	virtual int find(int) = 0;
	virtual void unify(int, int) = 0;

	virtual ~Core() = default;
};

} // namespace Unification
