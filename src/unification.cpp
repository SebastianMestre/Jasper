#include "unification.hpp"

#include <cassert>

namespace Unification {

template<>
int Core<>::new_var() {
	int id = header.size();
	header.push_back({Tag::Var, id});
	return id;
}

template<>
int Core<>::new_data() {
	int id = header.size();
	header.push_back({Tag::Data, data.size()});
	data.push_back(id);
	return id;
}

template<>
int Core<>::find(int id) {
	if (header[id].tag == Tag::Data or
	    header[id].equals == id)
		return id;

	return header[id].equals = find(header[id].equals);
}

template<>
void Core<>::unify(int a, int b) {
	a = find(a);
	b = find(b);

	if (a == b)
		return;

	if (header[a].tag == Tag::Var)
		header[a].equals = b;
	else if (header[b].tag == Tag::Var)
		header[b].equals = a;
	else
		assert(0 && "Tried to unify two different non-var values");
}

} // namespace Unification
