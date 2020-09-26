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

	virtual int new_var();
	virtual int new_data();

	virtual int find(int);
	virtual void unify(int, int);

	virtual ~Core() = default;
};

} // namespace Unification
